#include <iostream>
#include "StreamManager.hpp"
#include "Version.hpp"

static const std::string TOPIC = "[Main]";

int main() {
    StreamManager manager;

    std::cout << "--- Native MJPEG Streaming Runtime ---" << std::endl;
    std::cout << "Application Version: " << Version::FULL_STRING << std::endl;
    std::cout << "Commits since last tag: " << Version::COMMIT_COUNT << std::endl;
    std::cout << "Current Commit Hash: " << Version::GIT_HASH << std::endl;
    
    std::string prodPath = "/dev/v4l/by-id/usb-Jieli_Technology_USB_Composite_Device-video-index0";
    if(manager.createStream(prodPath, "0.0.0.0", 8001)){
        std::cout << TOPIC << " Stream initialized successfully" << std::endl;
    }

    std::cout << "[Main] Processing active feeds. Press Enter to cleanly terminate." << std::endl;
    std::cin.get();
    manager.stopAll();
    return 0;
}