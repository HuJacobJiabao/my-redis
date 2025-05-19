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
    // std::cout << "Parsing command: \n" << command << "\n";
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
    RedisDatabase& db = RedisDatabase::getInstance();

    if (cmd == "PING") {
        return handlePing(tokens, db);
    } else if (cmd == "ECHO") {
        return handleEcho(tokens, db);
    } else if (cmd == "FLUSHALL") {
        return handleFlushAll(tokens, db);
    } else if (cmd == "SET") {
        return handleSet(tokens, db);
    } else if (cmd == "GET") {
        return handleGet(tokens, db);
    } else if (cmd == "KEYS") {
        return handleKeys(tokens, db);
    } else if (cmd == "TYPE") {
        return handleType(tokens, db);
    } else if (cmd == "DEL" || cmd == "UNLINK") {
        return handleDel(tokens, db);
    } else if (cmd == "EXPIRE") {
        return handleExpire(tokens, db);
    } else if (cmd == "RENAME") {
        return handleRename(tokens, db);
    } else {
        return "-Error: Unknown command\r\n";
    }
}

// Handler function implementations
std::string handlePing(const std::vector<std::string>&, RedisDatabase&) {
    return "+PONG\r\n";
}

std::string handleEcho(const std::vector<std::string>& tokens, RedisDatabase&) {
    if (tokens.size() < 2)
        return "-Error: ECHO command requires a message\r\n";
    return "+" + tokens[1] + "\r\n";
}

std::string handleFlushAll(const std::vector<std::string>&, RedisDatabase& db) {
    db.flushAll();
    return "+OK\r\n";
}

std::string handleSet(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 3)
        return "-Error: SET command requires a key and a value\r\n";
    db.set(tokens[1], tokens[2]);
    return "+OK\r\n";
}

std::string handleGet(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 2)
        return "-Error: GET command requires a key\r\n";
    std::string value;
    if (db.get(tokens[1], value))
        return "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
    else
        return "$-1\r\n";
}

std::string handleKeys(const std::vector<std::string>&, RedisDatabase& db) {
    std::vector<std::string> allKeys = db.keys();
    std::ostringstream response;
    response << "*" << allKeys.size() << "\r\n";
    for (const auto& key : allKeys) {
        response << "$" << key.size() << "\r\n" << key << "\r\n";
    }
    return response.str();
}

std::string handleType(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 2)
        return "-Error: TYPE command requires a key\r\n";
    return "+" + db.type(tokens[1]) + "\r\n";
}

std::string handleDel(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 2)
        return "-Error: DEL command requires a key\r\n";
    bool res = db.del(tokens[1]);
    return ":" + std::to_string(res ? 1 : 0) + "\r\n";
}

std::string handleExpire(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 3)
        return "-Error: EXPIRE command requires a key and a time in seconds";
    int seconds = std::stoi(tokens[2]);
    if (db.expire(tokens[1], seconds))
        return "+OK\r\n";
    else
        return "-Error: Key not found\r\n";
}

std::string handleRename(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 3)
        return "-Error: RENAME command requires an old key name and a new key name";
    if (db.rename(tokens[1], tokens[2]))
        return "+OK\r\n";
    else
        return "-Error: RENAME failed\r\n";
}