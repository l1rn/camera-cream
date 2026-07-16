#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <string>
#include <unistd.h>

#define PORT 8001

void handle_client(int client_socket){
    char request_buffer[1024] = {0};
    ssize_t bytes_received = recv(client_socket, request_buffer, sizeof(request_buffer)- 1, 0);
    if (bytes_received < 0) {
        close(client_socket);
        return;
    }
    std::string response_header = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n"
        "Connection: keep-alive\r\n\r\n";
    if (send(client_socket, response_header.c_str(), response_header.length(), 0) < 0) {
        close(client_socket);
        return;
    }

    std::vector<uchar> buffer;
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 80};
    cv::Mat local_frame;

    while(camera_running){
        {
            std::lock_guard<std::mutex> lock(frame_mutex);
            if(!global_frame.empty()){
                global_frame.copyTo(local_frame);
            }
        }
        if(!local_frame.empty()){
            cv::imencode(".jpg", local_frame, buffer, params);
            std::string frame_header = 
                "--frame\r\n"
                "Content-Type: image/jpeg\r\n"
                "Content-Length: " + std::to_string(buffer.size()) + "\r\n\r\n";

            if (send(client_socket, frame_header.c_str(), frame_header.length(), 0) < 0) break;
            if (send(client_socket, reinterpret_cast<char*>(buffer.data()), buffer.size(), 0) < 0) break;
            if (send(client_socket, "\r\n", 2, 0) < 0) break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(30));

    }
    
    close(client_socket);
    std::cout << "Client disconnected." << std::endl;
}
int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == 0){
        std::cerr << "Socket creation failed." << std::endl;
        return -1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if(bind(server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0){
        std::cerr << "Port binding failed on port " << PORT << std::endl;
        close(server_fd);
        return -1;
    }

    if(listen(server_fd, 3) < 0){
        std::cerr << "Listen failed." << std::endl;
        close(server_fd);
        return -1;
    }

    std::cout << "MJPEG Streamer successfully listening at http://localhost:" << PORT << std::endl;
    std::thread(camera_producer).detach();
    while(true){
        int addrlen = sizeof(address);
        int client_socket = accept(server_fd, reinterpret_cast<sockaddr*>(&address), reinterpret_cast<socklen_t*>(&addrlen));
        if(client_socket < 0) continue;
        std::cout << "Client connected! Initializing stream pipeline..." << std::endl;
        std::thread(handle_client, client_socket).detach();
    }
    close(server_fd);
    return 0;
}