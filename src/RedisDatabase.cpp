#include "../include/RedisDatabase.h"
#include <iostream>
#include <sstream>
#include <fstream>

RedisDatabase& RedisDatabase::getInstance() {
    static RedisDatabase instance;
    return instance;
}

bool RedisDatabase::dump(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mtx); // Lock the mutex for thread safety
    std::cout << "Dumping database to " << filename << "\n";
    std::ofstream ofs(filename, std::ios::binary);
    
    if (!ofs) {
        std::cerr << "Error opening file for writing: " << filename << "\n";
        return false;
    }

    for (const auto& kv : kv_store) {
        ofs << "K" << kv.first << " " << kv.second << "\n";
    }

    for (const auto& kv : list_store) {
        ofs << "L" << kv.first << " ";
        for (const auto& item : kv.second) {
            ofs << " " << item;
        }
        ofs << "\n";
    }
        
    for (const auto& kv : hash_store) {
        ofs << "H" << kv.first << " ";
        for (const auto& item : kv.second) {
            ofs << " " << item.first << ":" << item.second;
        }
        ofs << "\n";
    }

    return true;
}

bool RedisDatabase::load(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mtx); // Lock the mutex for thread safety
    std::cout << "Loading database from " << filename << "\n";
    std::ifstream ifs(filename, std::ios::binary);

    if (!ifs) {
        std::cerr << "Error opening file for reading: " << filename << "\n";
        return false;
    }

    // Clear the existing data
    kv_store.clear();
    list_store.clear();
    hash_store.clear();

    std::string line;

    while (std::getline(ifs, line)) {
        std::istringstream iss(line);
        char type;
        iss >> type;

        if (type == 'K') {
            std::string key, value;
            iss >> key >> value;
            kv_store[key] = value;
        } else if (type == 'L') {
            std::string key;
            iss >> key;
            std::vector<std::string> list;
            std::string item;
            while (iss >> item) {
                list.push_back(item);
            }
            list_store[key] = list;
        } else if (type == 'H') {
            std::string key;
            iss >> key;
            std::unordered_map<std::string, std::string> hash;
            std::string pair;
            while (iss >> pair) {
                auto pos = pair.find(":");
                if (pos != std::string::npos) {
                    std::string field = pair.substr(0, pos);
                    std::string value = pair.substr(pos+1);
                    hash[field] = value;
                }
            }
            hash_store[key] = hash;
        }
        
    } 
    
    return true;
}

