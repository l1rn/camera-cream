#ifndef V4L2_CAMERA_HPP
#define V4L2_CAMERA_HPP

#include "CameraDevice.hpp"
#include <thread>
#include <mutex>
#include <atomic>
#include <string>
#include <vector>


struct CameraBuffer {
    void *start;
    size_t len;
};

class V4L2Camera : public CameraDevice {
public:
    V4L2Camera(const std::string& device = "/dev/video0", int w = 640, int h = 480);
    ~V4L2Camera() override;

    bool start() override;
    void stop() override;
    bool isRunning() const override { return m_running; }
    std::vector<uint8_t> getLatestJPEG() override;

private:
    bool initDevice();
    void uninitDevice();
    void captureLoop();

    std::string m_devicePath;
    int m_fd{-1};
    int m_width;
    int m_height;

    std::vector<CameraBuffer> m_buffers;
    std::vector<uint8_t> m_latestJpeg;
    std::mutex m_frameMutex;
    std::atomic<bool> m_running {false};
    std::thread m_thread;
};

#endif // V4L2_CAMERA_HPP