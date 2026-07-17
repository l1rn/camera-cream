#ifndef STREAM_MANAGER_HPP
#define STREAM_MANAGER_HPP

#include <string>
#include <mutex>
#include <memory>
#include <unordered_map>

#include "CameraDevice.hpp"
#include "StreamServer.hpp"

class StreamManager {
public:
    StreamManager() = default;
    ~StreamManager();

    StreamManager(const StreamManager&) = delete;
    StreamManager& operator=(const StreamManager&) = delete;
    bool createStream(const std::string &devicePath, const std::string &addr, int port);
    bool removeStream(int port);
    void stopAll();

private:
    std::mutex m_managerMutex;
    std::unordered_map<std::string, std::shared_ptr<CameraDevice>> m_cameras;
    std::unordered_map<int, std::unique_ptr<StreamServer>> m_servers;
};

#endif // STREAM_MANAGER_HPP