#include "../include/RedisServer.h"
#include "../include/RedisCommandHandler.h"
#include "../include/RedisDatabase.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <thread>
#include <cstring>
#include <csignal>


static RedisServer* globalServer = nullptr;

void signalHandler(int signum) {
    if (globalServer) {
        std::cout << "\nCaught Singal: " << signum << ", shutting down\n";
        std::cout.flush();
        globalServer->shutdown(); 
    }
    exit(signum);
}

void RedisServer::setupSignalHandler(){
    signal(SIGINT, signalHandler);
}

RedisServer::RedisServer(int port) : port(port), server_fd(-1), running(true) {
    globalServer = this;
    setupSignalHandler();
}

void RedisServer::shutdown() {
    running = false;
    if (server_fd != -1) {
        if (RedisDatabase::getInstance().dump("dump.my_rdb")) {
            std::cout << "Database dumped to dump.my_rdb successfully\n";
        } else {
            std::cerr << "Error dumping database\n";
        }
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

    std::vector<std::thread> threads;
    RedisCommandHandler cmdHandler;

    while (running) {
        int client_fd = accept(server_fd, nullptr, nullptr);
    
        threads.emplace_back([client_fd, &cmdHandler]() {
            char buffer[1024];
            while (true) {
                memset(buffer, 0, sizeof(buffer));
                int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
                if (bytes <= 0) break;
                
                std::string request(buffer, bytes);

                std::string response = cmdHandler.handleCommand(request);
                send(client_fd, response.c_str(), response.size(), 0);
            }
            // std::cout << "dbg HERE\n";
            close(client_fd);
        });
    }
    
    for (auto& t: threads){
        if (t.joinable()) t.join();
    }

    if (RedisDatabase::getInstance().dump("dump.my_rdb")) {
        std::cout << "Database dumped to dump.my_rdb successfully\n";
    } else {
        std::cerr << "Error dumping database\n";
    }

    // Shutdown

}