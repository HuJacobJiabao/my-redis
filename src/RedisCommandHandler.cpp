#include "../include/RedisCommandHandler.h"
#include "../include/RedisDatabase.h"
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>


// RESP parser
// Sample RESP "*2\r\n$5\r\nhello\r\n$5\r\nworld\r\n"
std::vector<std::string> parseRespCommand(const std::string& command) {
    std::vector<std::string> tokens;
    std::cout << "Parsing command: \n" << command << "\n";
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

    // std::cout << "crlf: " << crlf << "\n";

    if (crlf == std::string::npos) return tokens;

    int numElements = std::stoi(command.substr(pos, crlf - pos));

    // std::cout << "Number of elements: " << numElements << "\n";

    pos = crlf + 2;

    for (int i = 0; i < numElements; i++) {

        if (pos >= command.size() || command[pos] != '$') break; // format error
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

RedisCommandHandler::RedisCommandHandler(){}

std::string RedisCommandHandler::handleCommand(const std::string& command) {
    auto tokens = parseRespCommand(command);
    if (tokens.empty()) return "-Error: Empty command\r\n";

    std::string cmd = tokens[0];

    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
    std::ostringstream response;
    RedisDatabase& db = RedisDatabase::getInstance();

    // Handle commands
    if (cmd == "PING") {
        response << "+PONG\r\n";
    } else if (cmd == "ECHO") {
        if (tokens.size() < 2) 
            response << "-Error: ECHO command requires a message\r\n";
        else {
            response << "+" << tokens[1] << "\r\n";
        }
    } else if (cmd == "FLUSHALL") {
        db.flushAll();
        response << "+OK\r\n";
    } 
    // Key-value commands
    else if (cmd == "SET"){
        if (tokens.size() < 3) 
            response << "-Error: SET command requires a key and a value\r\n";
        else {
            std::string key = tokens[1];
            std::string value = tokens[2];
            if (db.set(key, value))
                response << "+OK\r\n";
        }
    } else if (cmd == "GET") {
        if (tokens.size() < 2) 
            response << "-Error: GET command requires a key\r\n";
        else {
            std::string value;
            if (db.get(tokens[1], value)) 
                response << "$" << value.size() << "\r\n" << value << "\r\n";
            else 
                response << "%-1\r\n";
        }
    } else if (cmd == "KEYS") {
        std::vector<std::string> allKeys = db.keys();
        response << "*" << allKeys.size() << "\r\n";
        for (const auto& key :allKeys) {
            response << "$" << key.size() << "\r\n" << key << "\r\n";
        }
    } else if (cmd == "TYPE") {
        if (tokens.size() < 2)
            response << "-Error: TYPE command requires a key\r\n";
        else {
            response << "+" << db.type(tokens[1]) << "\r\n";
        }
    } else if (cmd == "DEL" || cmd == "UNLINK") {
        if (tokens.size() < 2)
            response << "-Error: "<< cmd << " command requires a key\r\n";
        else {
            bool res = db.del(tokens[1]);
            response << ":" << (res ? 1 : 0) << "\r\n";
        }
    } else if (cmd == "EXPIRE") {
        if (tokens.size() < 3)
            response << "-Error: EXPIRE command requires a key and a time in seconds";
        else {
            if (db.expire(tokens[1], tokens[2]))
                //TODO: response needs to be corrected
                response << "+OK\r\n";
        }
    } else if (cmd == "RENAME") {
        if (tokens.size() < 3) 
            response << "-Error: RENAME command requires an old key name and a new key name";
        else {
            if (db.rename(tokens[1], tokens[2]))
                response << "+OK\r\n";
        }
    }
    else {
        response << "-Error: Unknown command\r\n";
    }

    return response.str();
}