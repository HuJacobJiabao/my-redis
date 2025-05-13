#include "../include/RedisServer.h"
#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {
    // int port = 6379; // Default port number for Redis
    int port = 45812;
    if (argc >= 2) port = std::stoi(argv[1]);
    
    RedisServer server(port);
    // Background persistance: dump the database every 300 seconds. (5 * 60 save database)
    std::thread persistanceThread([](){
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(300));
        }
    });
    persistanceThread.detach();

    server.run();

    return 0;
}