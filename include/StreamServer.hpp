#ifndef STREAM_SERVER_HPP
#define STREAM_SERVER_HPP

#include <string>
#include "CameraDevice.hpp"
#include <memory>
#include <atomic>
#include <thread>

class StreamServer {
public:
    StreamServer(std::shared_ptr<CameraDevice> camera, const std::string &addr, int port);
    ~StreamServer();

    bool start();
    void stop();

    int getPort() const;
    std::string getAddress();
    
private:
    void listenLoop();
    void handleClient(int clientSocket);

    std::shared_ptr<CameraDevice> m_camera;
    std::string m_address;
    int m_port;
    int m_serverFd{-1};

    std::atomic<bool> m_running{false};
    std::thread m_listenThread;
    std::vector<std::thread> m_clientThread;
};

#endif //STREAM_SERVER_HPP