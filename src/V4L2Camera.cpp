#include "V4L2Camera.hpp"
#include <iostream>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

V4L2Camera::V4L2Camera(const std::string& devicePath, int w, int h)
    : m_devicePath(devicePath), m_width(w), m_height(h) {}

V4L2Camera::~V4L2Camera(){
    stop();
}

bool V4L2Camera::initDevice(){
    m_fd = open(m_devicePath.c_str(), O_RDWR | O_NONBLOCK, 0);
    if (m_fd < 0){
        std::cerr << "Cannot open " << m_devicePath << std::endl;
        return false;
    }

    struct v4l2_format fmt{};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = m_width;
    fmt.fmt.pix.height = m_height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if (ioctl(m_fd, VIDIOC_S_FMT, &fmt) < 0){
        std::cerr << "Failed to set pixel format to mjpeg" << std::endl;
        return false;
    }

    struct v4l2_requestbuffers req{};
    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(m_fd, VIDIOC_REQBUFS, &req) < 0){
        std::cerr << "Buffer request failed" << std::endl;
        return false;
    }

    m_buffers.resize(req.count);
    for(size_t i = 0; i < req.count; ++i){
        struct v4l2_buffer buf{};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        if(ioctl(m_fd, VIDIOC_QUERYBUF, &buf) < 0) return false;

        m_buffers[i].len = buf.length;
        m_buffers[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, buf.m.offset);

        if(m_buffers[i].start == MAP_FAILED){
            std::cerr << "mmap failed." << std::endl;
            return false;
        }

        if (ioctl(m_fd, VIDIOC_QBUF, &buf) < 0) return false;
    }

    return true;
}

void V4L2Camera::uninitDevice(){
    for(auto& buf : m_buffers){
        munmap(buf.start, buf.len);
    }
    m_buffers.clear();
    if(m_fd >= 0){
        close(m_fd);
        m_fd = -1;
    }
}

bool V4L2Camera::start(){
    if(!initDevice()) return false;
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if(ioctl(m_fd, VIDIOC_STREAMON, &type) < 0){
        std::cerr << "Failed to start streaming." << std::endl;
        return false;
    }

    m_running = true;
    m_thread = std::thread(&V4L2Camera::captureLoop, this);
    return true;
}

void V4L2Camera::stop(){
    if(m_running){
        m_running = false;
        if(m_thread.joinable()){
            m_thread.join();
        }

        if(m_fd >= 0){
            enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            ioctl(m_fd, VIDIOC_STREAMOFF, &type);
        }

        uninitDevice();
    }
}

std::vector<uint8_t> V4L2Camera::getLatestJPEG(){
    std::lock_guard<std::mutex> lock(m_frameMutex);
    if(m_latestJpeg.empty()){
        return {};
    }
    return m_latestJpeg;
}

void V4L2Camera::captureLoop(){
    std::cout << "[V4L2Camera] Capture thread started successfully." << std::endl;
    while(m_running){
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(m_fd, &fds);
        struct timeval tv{};

        tv.tv_sec = 2;
        int r = select(m_fd + 1, &fds, NULL, NULL, &tv);
        
        if (r == 0) {
            std::cerr << "[V4L2Camera] Warning: select() timed out! Hardware is not sending frames." << std::endl;
            continue;
        } else if (r < 0) {
            std::cerr << "[V4L2Camera] Error: select() encountered a system error." << std::endl;
            continue;
        }

        struct v4l2_buffer buf{};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if (ioctl(m_fd, VIDIOC_DQBUF, &buf) < 0) {
            std::cerr << "[V4L2Camera] Failed to dequeue buffer from kernel." << std::endl;
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(m_frameMutex);
            uint8_t* rawData = static_cast<uint8_t*>(m_buffers[buf.index].start);
            m_latestJpeg.assign(rawData, rawData + buf.bytesused); 
        }

        ioctl(m_fd, VIDIOC_QBUF, &buf);
    }
    std::cout << "[V4L2Camera] Capture thread stopped." << std::endl;
}