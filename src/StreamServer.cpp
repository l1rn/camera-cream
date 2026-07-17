#include "StreamServer.hpp"
#include <iostream>
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #define close_socket(s) closesocket(s)
    using socket_size_t = int;
    using ssize_t = int;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define close_socket(s) close(s)
    using socket_size_t = socklen_t;
#endif // _WIN32 

StreamServer::StreamServer(std::shared_ptr<CameraDevice> camera, const std::string &addr, int port)
    : m_camera(camera), m_address(addr), m_port(port) {}

StreamServer::~StreamServer(){
    stop();
}
bool StreamServer::start() {
    if (m_running) return true;

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[Server " << m_port << "] Winsock initialization failed." << std::endl;
        return false;
    }
#endif

    m_serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_serverFd < 0) {
        std::cerr << "[Server " << m_port << "] Socket creation failed." << std::endl;
        return false;
    }

    int opt = 1;
#ifdef _WIN32
    setsockopt(m_serverFd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));
#else
    setsockopt(m_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    
    if (m_address == "0.0.0.0" || m_address.empty()) {
        addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        inet_pton(AF_INET, m_address.c_str(), &addr.sin_addr);
    }

    if (bind(m_serverFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "[Server " << m_port << "] Port binding failed." << std::endl;
        close_socket(m_serverFd);
        return false;
    }

    if (listen(m_serverFd, 10) < 0) {
        std::cerr << "[Server " << m_port << "] Listen failed." << std::endl;
        close_socket(m_serverFd);
        return false;
    }

    m_running = true;
    m_listenThread = std::thread(&StreamServer::listenLoop, this);
    
    std::cout << "[Server " << m_port << "] Native passthrough active at http://" 
              << (m_address.empty() ? "localhost" : m_address) << ":" << m_port << std::endl;
    return true;
}

void StreamServer::stop() {
    if (m_running) {
        m_running = false;
        
        if (m_serverFd >= 0) {
            close_socket(m_serverFd);
            m_serverFd = -1;
        }

        if (m_listenThread.joinable()) {
            m_listenThread.join();
        }

        for (auto &t : m_clientThread) {
            if (t.joinable()) {
                t.join();
            }
        }
        m_clientThread.clear();

#ifdef _WIN32
        WSACleanup();
#endif
    }
}

int StreamServer::getPort() const {
    return m_port;
}

std::string StreamServer::getAddress(){
    return m_address;
}

void StreamServer::listenLoop(){
    while (m_running)
    {
        sockaddr_in client{};
        socket_size_t addrLen = sizeof(client);
        int clientSocket = accept(m_serverFd, reinterpret_cast<sockaddr*>(&client), &addrLen);
        if(clientSocket < 0){
            if(m_running) continue;
            else break;
        }

        m_clientThread.push_back(std::thread(&StreamServer::handleClient, this, clientSocket));
    }
}

void StreamServer::handleClient(int clientSocket){
    char request_buffer[1024] = {0};
    ssize_t bytes_received = recv(clientSocket, request_buffer, sizeof(request_buffer)- 1, 0);
    if (bytes_received < 0) {
        close(clientSocket);
        return;
    }
    std::string response_header = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n"
        "Connection: keep-alive\r\n\r\n";
    if (send(clientSocket, response_header.c_str(), response_header.length(), 0) < 0) {
        close(clientSocket);
        return;
    }

    while(m_running && m_camera && m_camera->isRunning()){
        std::vector<uint8_t> jpeg_bytes = m_camera->getLatestJPEG();
        if(!jpeg_bytes.empty()){
            std::string frame_header = 
                "--frame\r\n"
                "Content-Type: image/jpeg\r\n"
                "Content-Length: " + std::to_string(jpeg_bytes.size()) + "\r\n\r\n";

            if (send(clientSocket, frame_header.c_str(), frame_header.length(), 0) < 0) break;
            if (send(clientSocket, reinterpret_cast<char*>(jpeg_bytes.data()), jpeg_bytes.size(), 0) < 0) break;
            if (send(clientSocket, "\r\n", 2, 0) < 0) break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(30));

    }
    
    close_socket(clientSocket);
    std::cout << "Client disconnected." << std::endl;
}
