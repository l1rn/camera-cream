#include "StreamManager.hpp"
#include <iostream>

#ifdef _WIN32
    // win.hpp
#else 
    #include "V4L2Camera.hpp"
#endif //

static const std::string TOPIC = "[StreamManager]";  

StreamManager::~StreamManager(){
    stopAll();
}

bool StreamManager::createStream(int deviceIndex, const std::string &addr, int port){
    std::lock_guard<std::mutex> lock(m_managerMutex);
    if(m_servers.find(port) != m_servers.end()){
        std::cerr << TOPIC << " Error: Port " << port << " is already running an active stream" << std::endl;
        return false;
    }

    std::shared_ptr<CameraDevice> camera;
    auto camIt = m_cameras.find(deviceIndex);

    if(camIt != m_cameras.end()){
        camera = camIt->second;
        std::cout << TOPIC << " Reusing active camera hardware backend for index: " << deviceIndex << std::endl;
    } else {
#ifdef _WIN32
        return false;
#else
        std::string devPath = "/dev/video" + std::to_string(deviceIndex);
        camera = std::make_shared<V4L2Camera>(devPath);
#endif
        if(!camera->start()){
            std::cerr << TOPIC << " Failed to start camera driver for device: " << deviceIndex << std::endl;
            return false;
        }
    }
    m_cameras[deviceIndex] = camera;
    auto server = std::make_unique<StreamServer>(camera, addr, port);
    if(!server->start()){
        std::cerr << TOPIC << " Failed to bind network socket server on port: " << port << std::endl;
        if(camera.use_count() <= 2){
            camera->stop();
            m_cameras.erase(deviceIndex);
        }
        return false;
    }

    m_servers[port] = std::move(server);
    return true;
}

bool StreamManager::removeStream(int port){
    std::lock_guard<std::mutex> lock(m_managerMutex);

    auto sIt = m_servers.find(port);
    if(sIt == m_servers.end()){
        std::cerr << TOPIC << " Warning: No active server found on port " << port << std::endl;
        return false;
    }

    std::cout << TOPIC << " Killing stream server instance on port " << port << std::endl;
    sIt->second->stop();
    m_servers.erase(sIt);

    auto camIt = m_cameras.begin();
    while(camIt != m_cameras.end()){
        if(camIt->second.use_count() == 1){
            std::cout << TOPIC << " Device index " << camIt->first << " has no active clients. Powering down hardware driver..." << std::endl;
            camIt->second->stop();
            camIt = m_cameras.erase(camIt);
        } else {
            ++camIt;
        }
    }

    return true;
}

void StreamManager::stopAll(){
    std::lock_guard<std::mutex> lock(m_managerMutex);
    std::cout << TOPIC << "Executing global pipeline shutdown sequence..." << std::endl;
    for(auto& pair : m_servers){
        pair.second->stop();
    }
    m_servers.clear();
    
    for(auto& pair : m_cameras){
        pair.second->stop();
    }
    m_cameras.clear();
    std::cout << TOPIC << " All systems clear." << std::endl;
}