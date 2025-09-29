CXX = g++

CXXFLAGS = -std=c++11 -Wall -Wextra -g

TARGET = CameraManagerApp

SRCS = $(wildcard *.cpp)

OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)	

%.o: %.cpp	
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)	
	sudo ./$(TARGET)

clean:	
	rm -f $(OBJS) $(TARGET)	

.PHONY: all run clean
