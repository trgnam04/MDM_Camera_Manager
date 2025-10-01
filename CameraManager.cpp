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
public:        
    void setCameraState(bool enable) {
        if (enable) {
            int ret = system("chmod 660 /dev/video*");
            if(ret == -1)
            {
                std::cerr << "[MGM][ERROR] Failed to change camera permissions." << std::endl;
            }
        }
        else {
            int ret = system("chmod 000 /dev/video*");
            if (ret == -1)
            {
                std::cerr << "[MGM][ERROR] Failed to change camera permissions." << std::endl;
            }
            
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
