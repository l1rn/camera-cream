#include "CameraDevice.hpp"
#include <iostream>

CameraDevice::CameraDevice(int dI, int w, int h, int fps) 
    : m_deviceIndex(dI), m_width(w), m_height(h),m_fps(fps) {}

CameraDevice::~CameraDevice(){
    stop();
}

bool CameraDevice::start(){
    if(m_running) return true;
    m_running = true;
    m_thread = std::thread(&CameraDevice::captureLoop, this);
    return true;
}

void CameraDevice::stop(){
    if(m_running){
        m_running = false;
        if(m_thread.joinable()){
            m_thread.join();
        }
    }
}

bool CameraDevice::isRunning() const {
    return m_running;
}

int CameraDevice::getDeviceIndex() const {
    return m_deviceIndex;
}

cv::Mat CameraDevice::getLatestFrame(){
    std::lock_guard<std::mutex> lock(m_frameMutex);
    return m_latestFrame.clone();
}

void CameraDevice::captureLoop(){
    cv::VideoCapture cap(0, cv::CAP_V4L2);

    if(!cap.isOpened()) {
        std::cerr << "Error: unable to open webcam interface." << std::endl;
        m_running = false;
        return;
    }

    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    cap.set(cv::CAP_PROP_FPS, 30);
    
    cv::Mat temp_frame;
    while(m_running){
        cap >> temp_frame;
        if(temp_frame.empty()){
            std::cerr << "Warning: Captured empty frame, retrying..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        {
            std::lock_guard<std::mutex> lock(m_frameMutex);
            temp_frame.copyTo(m_latestFrame);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    cap.release();
}