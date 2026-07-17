#ifndef CAMERA_DEVICE_HPP
#define CAMERA_DEVICE_HPP

#include <vector>
#include <cstdint>

class CameraDevice {
public:
    virtual ~CameraDevice() = default;
    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual bool isRunning() const = 0;
    virtual std::vector<uint8_t> getLatestJPEG() = 0;
};

#endif // CAMERA_DEVICE_HPP
