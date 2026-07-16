#include "StreamServer.hpp"

StreamServer::StreamServer(std::shared_ptr<CameraDevice> camera, const std::string &addr, int port)
    : m_camera(camera), m_address(addr), m_port(port) {}

StreamServer::~StreamServer(){
    stop();
}

