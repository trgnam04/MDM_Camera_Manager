#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <dirent.h>   
#include <unistd.h>   
#include <sys/types.h> 

#include "json.hpp"

using json = nlohmann::json;

class CameraManager {
private:
    bool cameraPolicy;

    void loadPolicy(const std::string& filename) {
        std::ifstream configFile(filename);
        if (!configFile.is_open()) {
            throw std::runtime_error("[MGM][ERROR] Cannot open configuration file: " + filename);
        }
        try {   
            json config = json::parse(configFile);
            bool ruleFound = false;
            for (const auto& policy : config["policies"]) {
                if (policy["key"] == "connectivity.camera") {
                    cameraPolicy = policy["value"].get<bool>();
                    ruleFound = true;
                    break;
                }
            }
            if (!ruleFound) {
                throw std::runtime_error("[MGM][ERROR] Policy 'connectivity.camera' not found in " + filename);
            }
        } catch (json::parse_error& e) {
            throw std::runtime_error("[MGM][ERROR] JSON parsing failed: " + std::string(e.what()));
        }
    }

public:
    CameraManager(const std::string& profilePath) {
        try {
            loadPolicy(profilePath);
            std::cout << "[MGM][INFO] Policy loaded successfully: Camera is set to " 
                      << (cameraPolicy ? "ENABLED" : "DISABLED") << std::endl;
        } catch (const std::runtime_error& e) {
            std::cerr << e.what() << std::endl;
            cameraPolicy = false; 
            std::cerr << "[MGM][WARN] Defaulting camera policy to DISABLED for safety." << std::endl;
        }
    }

    bool isCameraAllowed() const {
        return cameraPolicy;
    }
};


class HardwareController {
private:
    const std::string TARGET_VENDOR_ID = "13d3";
    const std::string TARGET_PRODUCT_ID = "56eb";

    std::string readFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return "";
        std::string content;
        file >> content;
        return content;
    }
    
    std::string findCameraDevicePath() {
        const std::string basePath = "/sys/bus/usb/devices/";
        DIR* dir = opendir(basePath.c_str());
        if (!dir) {
            perror("[HW][ERROR] Cannot open /sys/bus/usb/devices");
            return "";
        }

        std::cout << "[HW][INFO] Searching for camera with VendorID=" << TARGET_VENDOR_ID 
                  << " and ProductID=" << TARGET_PRODUCT_ID << " ..." << std::endl;

        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            std::string deviceName = entry->d_name;
            if (deviceName == "." || deviceName == "..") continue;

            std::string devicePath = basePath + deviceName;
            std::string vendorIdPath = devicePath + "/idVendor";
            std::string productIdPath = devicePath + "/idProduct";
            
            if (readFile(vendorIdPath) == TARGET_VENDOR_ID && readFile(productIdPath) == TARGET_PRODUCT_ID) {
                std::cout << "[HW][INFO] Camera device detected at: " << devicePath << std::endl;
                closedir(dir);
                return devicePath;
            }
        }

        closedir(dir);
        std::cerr << "[HW][ERROR] No matching camera device found!" << std::endl;
        return "";
    }


public:    
    void setCameraState(bool enable) {
        std::string devicePath = findCameraDevicePath();
        if (devicePath.empty()) {
            std::cerr << "[HW][WARN] Camera state cannot be changed because device was not found." << std::endl;
            return;
        }

        std::string authorizedPath = devicePath + "/authorized";
        std::ofstream authorizedFile(authorizedPath);

        if (!authorizedFile.is_open()) {
            std::cerr << "[HW][ERROR] Cannot open file: " << authorizedPath 
                      << ". Did you run with 'sudo' privileges?" << std::endl;
            return;
        }

        const char* value = enable ? "1" : "0";
        authorizedFile << value;
        authorizedFile.close();

        if (authorizedFile.fail()) {
             std::cerr << "[HW][ERROR] Failed to write to file: " << authorizedPath << std::endl;
        } else {
            std::cout << "[HW][INFO] Camera has been successfully " << (enable ? "ENABLED" : "DISABLED") << "." << std::endl;
        }
    }
};


int main() {    
    if (geteuid() != 0) {
        std::cerr << "[MAIN][ERROR] This program requires root privileges. Please run with 'sudo'." << std::endl;
        return 1;
    }
    
    CameraManager mgm("profile.json");
    bool shouldBeEnabled = mgm.isCameraAllowed();
    
    HardwareController controller;
    controller.setCameraState(shouldBeEnabled);
    
    std::cout << "[MAIN][INFO] Operation completed. Final camera state request: " 
              << (shouldBeEnabled ? "ENABLED" : "DISABLED") << std::endl;

    return 0;
}
