#ifndef REDIS_COMMAND_HANDLER_H
#define REDIS_COMMAND_HANDLER_H

#include <string>

class RedisCommandHandler {
public:
    RedisCommandHandler();
    std::string handleCommand(const std::string& command);
};

#endif