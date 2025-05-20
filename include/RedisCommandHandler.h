#ifndef REDIS_COMMAND_HANDLER_H
#define REDIS_COMMAND_HANDLER_H

#include <string>
#include <vector>
#include "RedisDatabase.h"

class RedisCommandHandler {
public:
    RedisCommandHandler();
    std::string handleCommand(const std::string& command);
};

// Common commands
// Handles the PING command. Returns "+PONG".
std::string handlePing(const std::vector<std::string>& tokens, RedisDatabase& db);
// Handles the ECHO command. Returns the message sent by the client.
std::string handleEcho(const std::vector<std::string>& tokens, RedisDatabase& db);
// Handles the FLUSHALL command. Clears the database.
std::string handleFlushAll(const std::vector<std::string>& tokens, RedisDatabase& db);

// Key/Value operations
// Handles the SET command. Sets the value of a key.
std::string handleSet(const std::vector<std::string>& tokens, RedisDatabase& db);
// Handles the GET command. Gets the value of a key.
std::string handleGet(const std::vector<std::string>& tokens, RedisDatabase& db);
// Handles the KEYS command. Returns all keys in the database.
std::string handleKeys(const std::vector<std::string>& tokens, RedisDatabase& db);
// Handles the TYPE command. Returns the type of the value stored at key.
std::string handleType(const std::vector<std::string>& tokens, RedisDatabase& db);
// Handles the DEL/UNLINK command. Deletes a key.
std::string handleDel(const std::vector<std::string>& tokens, RedisDatabase& db);
// Handles the EXPIRE command. Sets a timeout on a key.
std::string handleExpire(const std::vector<std::string>& tokens, RedisDatabase& db);
// Handles the RENAME command. Renames a key.
std::string handleRename(const std::vector<std::string>& tokens, RedisDatabase& db);

// List operations
// Handles the LLEN command. Returns the length of a list.
std::string handleLlen(const std::vector<std::string>& tokens, RedisDatabase& db);
// Handles the LPUSH command. Inserts values at the head of a list.
std::string handleLpush(const std::vector<std::string>& tokens, RedisDatabase& db);
// Handles the RPUSH command. Inserts values at the tail of a list.
std::string handleRpush(const std::vector<std::string>& tokens, RedisDatabase& db);
// Handles the LPOP command. Removes and returns the first element of a list.
std::string handleLpop(const std::vector<std::string>& tokens, RedisDatabase& db);
// Handles the RPOP command. Removes and returns the last element of a list.
std::string handleRpop(const std::vector<std::string>& tokens, RedisDatabase& db);
// Handles the LREM command. Removes elements from a list.
std::string handleLrem(const std::vector<std::string>& tokens, RedisDatabase& db);
// Handles the LINDEX command. Gets an element from a list by its index.
std::string handleLindex(const std::vector<std::string>& tokens, RedisDatabase& db);
// Handles the LSET command. Sets the value of an element in a list by its index.
std::string handleLset(const std::vector<std::string>& tokens, RedisDatabase& db);

#endif