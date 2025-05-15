#include <string>
#ifndef REDIS_DATABASE_H
#define REDIS_DATABASE_H

class RedisDatabase {
public:
    // Get the singleton instance
    static RedisDatabase& getInstance();

    // Persistance: dump / load the database from a file
    bool dump(const std::string& filename);
    bool load(const std::string& filename); 
private:
    RedisDatabase() = default;
    ~RedisDatabase() = default;
    RedisDatabase(const RedisDatabase&) = delete;
    RedisDatabase& operator = (const RedisDatabase&) = delete;
};

#endif