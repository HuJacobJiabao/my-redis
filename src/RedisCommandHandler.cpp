#include "../include/RedisCommandHandler.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>

// RESP parser
// Sample RESP "*2\r\n$5\r\nhello\r\n$5\r\nworld\r\n"
std::vector<std::string> parseRespCommand(const std::string& command) {
    std::vector<std::string> tokens;
    if (command.empty()) return tokens;

    // If it doesn't start with '*', fallback to splitting by whitespace.
    if (command[0] != '*') {
        std::istringstream iss(command);
        std::string token;
        while (iss >> token) {
            tokens.push_back(token);
        }
        return tokens;
    }

    size_t pos = 0;

    // Expect '*' followed by number of elements
    if (command[pos] != '*') return tokens;
    pos++;

    // crlf = carriage return (\r) + line feed (\n)
    size_t crlf = command.find("\r\n", pos);
    if (crlf == std::string::npos) return tokens;
    
    int numElements = std::stoi(command.substr(pos, crlf - pos));
    pos = crlf + 2;

    for (int i = 0; i < numElements; i++) {
        if (pos >= command.size() || command[pos] != '&') break; // format error
        pos++;  // skip '$'
        
        crlf = command.find("\r\n", pos);
        if (crlf == std::string::npos) break;
        int len = std::stoi(command.substr(pos, crlf - pos));
        pos = crlf + 2;

        if (pos + len > command.size()) break;
        std::string token = command.substr(pos,len);
        tokens.push_back(token);
        pos += len + 2;
    }

    return tokens;

}

std::string RedisCommandHandler::handleCommand(const std::string& command) {
    auto tokens = parseRespCommand(command);
    if (tokens.empty()) return "-Error: Empty command\r\n";

    std::string cmd = tokens[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
    std::ostringstream response;


    return response.str();
}