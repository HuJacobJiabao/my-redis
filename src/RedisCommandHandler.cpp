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

    // Common commands
    if (cmd == "PING") {
        return handlePing(tokens, db);
    } else if (cmd == "ECHO") {
        return handleEcho(tokens, db);
    } else if (cmd == "FLUSHALL") {
        return handleFlushAll(tokens, db);
    } 
    // Key/Value operations
    else if (cmd == "SET") {
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
    }  
    // List operations
    else if (cmd == "LLEN") {
        return handleLlen(tokens, db);
    } else if (cmd == "LPUSH") {
        return handleLpush(tokens, db);
    } else if (cmd == "RPUSH") {
        return handleRpush(tokens, db);
    } else if (cmd == "LPOP") {
        return handleLpop(tokens, db);
    } else if (cmd == "RPOP") {
        return handleRpop(tokens, db);
    } else if (cmd == "LREM") {
        return handleLrem(tokens, db);
    } else if (cmd == "LINDEX") {
        return handleLindex(tokens, db);
    } else if (cmd == "LSET") {
        return handleLset(tokens, db);
    } else if (cmd == "HSET") {
        return handleHset(tokens, db);
    } else if (cmd == "HGET") {
        return handleHget(tokens, db);
    } else if (cmd == "HEXISTS") {
        return handleHexists(tokens, db);
    } else if (cmd == "HDEL") {
        return handleHdel(tokens, db);
    } else if (cmd == "HGETALL") {
        return handleHgetall(tokens, db);
    } else if (cmd == "HKEYS") {
        return handleHkeys(tokens, db);
    } else if (cmd == "HVALS") {
        return handleHvals(tokens, db);
    } else if (cmd == "HLEN") {
        return handleHlen(tokens, db);
    } else if (cmd == "HMSET") {
        return handleHmset(tokens, db);
    }
    else {
        return "-Error: Unknown command\r\n";
    }
}

// *** Handler function implementations ***

// Common functions
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

// Key/Value operations
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

// List Operations
std::string handleLlen(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 2)
        return "-Error: LLEN command requires key\r\n";
    ssize_t len = db.llen(tokens[1]);
    return ":" + std::to_string(len) + "\r\n";
}

std::string handleLpush(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 3)
        return "-Error: LPUSH command requires key and at least one value\r\n";
    std::vector<std::string> values(tokens.begin() + 2, tokens.end());
    db.lpush(tokens[1], values);
    ssize_t len = db.llen(tokens[1]);
    return ":" + std::to_string(len) + "\r\n";
}

std::string handleRpush(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 3)
        return "-Error: RPUSH command requires key and at least one value\r\n";
    std::vector<std::string> values(tokens.begin() + 2, tokens.end());
    db.rpush(tokens[1], values);
    ssize_t len = db.llen(tokens[1]);
    return ":" + std::to_string(len) + "\r\n";
}

std::string handleLpop(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 2)
        return "-Error: LPOP command requires key\r\n";
    std::string val;
    if (db.lpop(tokens[1], val))
        return "$" + std::to_string(val.size()) + "\r\n" + val + "\r\n";
    return "$-1\r\n";
}
 
std::string handleRpop(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 2)
        return "-Error: RPOP command requires key\r\n";
    std::string val;
    if (db.rpop(tokens[1], val))
        return "$" + std::to_string(val.size()) + "\r\n" + val + "\r\n";
    return "$-1\r\n";
}

std::string handleLrem(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 4)
        return "-Error: LREM command requires key, count, and value\r\n";
    try {
        int count = std::stoi(tokens[2]);
        int removed = db.lrem(tokens[1], count, tokens[3]);
        return ":" + std::to_string(removed) + "\r\n";
    } catch (const std::exception&) {
        return "-Error: Invalid count\r\n";
    }
}

std::string handleLindex(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 3)
        return "-Error: LINDEX command requires key and index\r\n";
    try {
        int index = std::stoi(tokens[2]);
        std::string value;
        if (db.lindex(tokens[1], index, value))
            return "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
        else
            return "$-1\r\n";
    } catch (const std::exception&) {
        return "-Error: Invalid index\r\n";
    }
}

std::string handleLset(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 4)
        return "-Error: LSET command requires key, index, and value\r\n";
    try {
        int index = std::stoi(tokens[2]);
        if (db.lset(tokens[1], index, tokens[3]))
            return "+OK\r\n";
        else
            return "-Error: Index out or range\r\n";
    } catch (const std::exception&) {
        return "-Error: Invalid index\r\n";
    }
}

// Hash Operations
std::string handleHset(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 4)
        return "-Error: HSET requires key, field, and value\r\n";
    int updated = db.hset(tokens[1], tokens[2], tokens[3]);
    return ":" + std::to_string(updated) + "\r\n";
}

std::string handleHget(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 3)
        return "-Error: HGET requires key and field\r\n";
    std::string value;
    if (db.hget(tokens[1], tokens[2], value))
        return "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
    else
        return "$-1\r\n";
}

std::string handleHexists(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 3)
        return "-Error: HEXISTS requires key and field\r\n";
    bool exists = db.hexists(tokens[1], tokens[2]);
    return ":" + std::to_string(exists ? 1 : 0) + "\r\n";
}

std::string handleHdel(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 3)
        return "-Error: HDEL requires key and field\r\n";
    int removed = db.hdel(tokens[1], tokens[2]);
    return ":" + std::to_string(removed) + "\r\n";
}

std::string handleHgetall(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 2)
        return "-Error: HGETALL requires key\r\n";
    auto pairs = db.hgetall(tokens[1]);
    std::ostringstream resp;
    resp << "*" << pairs.size() * 2 << "\r\n";
    for (const auto& kv : pairs) {
        resp << "$" << kv.first.size() << "\r\n" << kv.first << "\r\n";
        resp << "$" << kv.second.size() << "\r\n" << kv.second << "\r\n";
    }
    return resp.str();
}

std::string handleHkeys(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 2)
        return "-Error: HKEYS requires key\r\n";
    auto keys = db.hkeys(tokens[1]);
    std::ostringstream resp;
    resp << "*" << keys.size() << "\r\n";
    for (const auto& k : keys) {
        resp << "$" << k.size() << "\r\n" << k << "\r\n";
    }
    return resp.str();
}

std::string handleHvals(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 2)
        return "-Error: HVALS requires key\r\n";
    auto vals = db.hvals(tokens[1]);
    std::ostringstream resp;
    resp << "*" << vals.size() << "\r\n";
    for (const auto& v : vals) {
        resp << "$" << v.size() << "\r\n" << v << "\r\n";
    }
    return resp.str();
}

std::string handleHlen(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 2)
        return "-Error: HLEN requires key\r\n";
    int len = db.hlen(tokens[1]);
    return ":" + std::to_string(len) + "\r\n";
}

std::string handleHmset(const std::vector<std::string>& tokens, RedisDatabase& db) {
    if (tokens.size() < 4 || (tokens.size() - 2) % 2 != 0)
        return "-Error: HMSET requires key and one or more field value pairs\r\n";
    std::vector<std::pair<std::string, std::string>> field_values;
    for (size_t i = 2; i + 1 < tokens.size(); i += 2) {
        field_values.emplace_back(tokens[i], tokens[i + 1]);
    }
    db.hmset(tokens[1], field_values);
    return "+OK\r\n";
}