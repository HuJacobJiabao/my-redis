#include "../include/RedisServer.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>


static RedisServer* globalServer = nullptr;

RedisServer::RedisServer(int port) : port(port), server_fd(-1), running(true) {
    globalServer = this;
}

void RedisServer::shutdown() {
    running = false;
    if (server_fd != -1) {
        close(server_fd);
    }
    std::cout << "Server shutdown complete\n";
}

void RedisServer::run() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Error creating server socket\n";
        return; 
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error binding server socket\n";
        return;
    }

    int connection_backlog = 10;
    if (listen(server_fd, connection_backlog) < 0) {
        std::cerr << "listen failed\n";
        return;
    }

    std::cout << "Redis Server Listening On Port: " << port << ".\n";
}