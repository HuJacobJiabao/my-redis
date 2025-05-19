#include <string>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <chrono>

#ifndef REDIS_DATABASE_H
#define REDIS_DATABASE_H

class RedisDatabase {
public:
    // Get the singleton instance
    static RedisDatabase& getInstance();

    bool flushAll();

    // Key-Value operations
    void set(const std::string& key, const std::string& value);
    bool get(const std::string& key, std::string& value);
    bool del(const std::string& key);
    bool exists(const std::string& key);
    std::vector<std::string> keys();
    std::string type(const std::string& key);
    bool expire(const std::string& key, int seconds);
    bool rename(const std::string& oldKey, const std::string& newKey);



    // Persistance: dump / load the database from a file
    bool dump(const std::string& filename);
    bool load(const std::string& filename); 
private:
    RedisDatabase() = default;
    ~RedisDatabase() = default;
    RedisDatabase(const RedisDatabase&) = delete;
    RedisDatabase& operator = (const RedisDatabase&) = delete;

    std::mutex mtx; // Mutex for thread safety
    std::unordered_map<std::string, std::string> kv_store; // In-memory key-value store
    std::unordered_map<std::string, std::vector<std::string>> list_store; // In-memory list store
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> hash_store; // In-memory hash store

    std::unordered_map<std::string, std::chrono::steady_clock::time_point> expiry_map;
};

#endif