#ifndef CAMERA_DEVICE_HPP
#define CAMERA_DEVICE_HPP

#include <opencv2/opencv.hpp>
#include <thread>
#include <mutex>
#include <atomic>

class CameraDevice{
public:
    CameraDevice(int dI, int w = 640, int h = 480, int fps = 30);
    ~CameraDevice();

    bool start();
    void stop();
    bool isRunning() const;
    int getDeviceIndex() const;
    cv::Mat getLatestFrame();

private:
    void captureLoop();

    int m_deviceIndex;
    int m_width;
    int m_height;
    int m_fps;

    std::atomic<bool> m_running{false};
    std::thread m_thread;
    std::mutex m_frameMutex;
    cv::Mat m_latestFrame;
};

#endif // CAMERA_DEVICE_HPP