#include "../include/RedisServer.h"
#include <iostream>


int main(int argc, char* argv[]) {
    // int port = 6379; // Default port number for Redis
    int port = 45812;
    if (argc >= 2) port = std::stoi(argv[1]);
    
    RedisServer server(port);
    server.run();

    return 0;
}