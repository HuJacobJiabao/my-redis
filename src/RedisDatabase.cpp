#include "../include/RedisDatabase.h"
#include <iostream>

RedisDatabase& RedisDatabase::getInstance() {
    static RedisDatabase instance;
    return instance;
}

bool RedisDatabase::dump(const std::string& filename) {
    // Simulate dumping the database to a file
    std::cout << "Dumping database to " << filename << "\n";
    return true;
}

bool RedisDatabase::load(const std::string& filename) {
    // Simulate loading the database from a file
    std::cout << "Loading database from " << filename << "\n";
    return true;
}

