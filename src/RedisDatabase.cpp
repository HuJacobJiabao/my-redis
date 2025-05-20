#include "../include/RedisDatabase.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

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
        ofs << "K " << kv.first << " " << kv.second << "\n";
    }

    for (const auto& kv : list_store) {
        ofs << "L " << kv.first << " ";
        for (const auto& item : kv.second) {
            ofs << " " << item;
        }
        ofs << "\n";
    }
        
    for (const auto& kv : hash_store) {
        ofs << "H " << kv.first << " ";
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

bool RedisDatabase::flushAll() {
    std::lock_guard<std::mutex> lock(mtx);
    kv_store.clear();
    list_store.clear();
    hash_store.clear();
    return true;
}

// Key-Value Operations
void RedisDatabase::set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mtx);
    kv_store[key] = value;
}

bool RedisDatabase::get(const std::string& key, std::string& value) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = kv_store.find(key);
    if (it != kv_store.end()) {
        value = it->second;
        return true;
    }
    return false;
}

bool RedisDatabase::del(const std::string& key) {
    std::lock_guard<std::mutex> lock(mtx);
    bool erased = false;
    erased |= kv_store.erase(key) > 0;
    erased |= list_store.erase(key) > 0;
    erased |= hash_store.erase(key) > 0;

    return erased;
}


std::vector<std::string> RedisDatabase::keys() {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<std::string> result;
    for (const auto& pair : kv_store){
        result.push_back(pair.first);
    }
    for (const auto& pair : list_store){
        result.push_back(pair.first);
    } 
    for (const auto& pair : hash_store){
        result.push_back(pair.first);
    }

    return result;
}

std::string RedisDatabase::type(const std::string& key) {
    std::lock_guard<std::mutex> lock(mtx);
    
    if (kv_store.find(key) != kv_store.end())   return "string";

    if (list_store.find(key) != list_store.end())   return "list";
    
    if (hash_store.find(key) != hash_store.end())   return "hash";
    
    else return "none";
}

bool RedisDatabase::expire(const std::string& key, int seconds) {
    std::lock_guard<std::mutex> lock(mtx);
    bool exist = kv_store.find(key) != kv_store.end() || 
                 list_store.find(key) != list_store.end() ||
                 hash_store.find(key) != hash_store.end();
    if (!exist) return false;

    expiry_map[key] = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
    
    return true;
}

bool RedisDatabase::rename(const std::string& oldKey, const std::string& newKey) {
    std::lock_guard<std::mutex> lock(mtx);
    bool found = false;

    auto itKv = kv_store.find(oldKey);
    if(itKv != kv_store.end()) {
        kv_store[newKey] = itKv -> second;
        kv_store.erase(itKv);
        found = true;
    }

    auto itList = list_store.find(oldKey);
    if(itList != list_store.end()) {
        list_store[newKey] = itList -> second;
        list_store.erase(itList);
        found = true;
    }

    auto itHash = hash_store.find(oldKey);
    if(itHash != hash_store.end()) {
        hash_store[newKey] = itHash -> second;
        hash_store.erase(itHash);
        found = true;
    }

    auto itExpiry = expiry_map.find(oldKey);
    if(itExpiry != expiry_map.end()) {
        expiry_map[newKey] = itExpiry -> second;
        expiry_map.erase(itExpiry);
    }

    return found;

}

// List Operations
ssize_t RedisDatabase::llen(const std::string& key) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = list_store.find(key);
    if (it != list_store.end())
        return it->second.size();
    return 0;
}

void RedisDatabase::lpush(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mtx);
    list_store[key].insert(list_store[key].begin(), value);
}

void RedisDatabase::rpush(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mtx);
    list_store[key].push_back(value);
}

bool RedisDatabase::lpop(const std::string& key, std::string& value) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = list_store.find(key);
    if (it != list_store.end() && !it->second.empty()) {
        value = it->second.front();
        it->second.erase(it->second.begin());
        return true;
    }

    return false;
}

bool RedisDatabase::rpop(const std::string& key, std::string& value) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = list_store.find(key);
    if (it != list_store.end() && !it->second.empty()) {
        value = it->second.back();
        it->second.pop_back();
        return true;
    }

    return false;
}

int RedisDatabase::lrem(const std::string& key, int count, const std::string& value) {
    std::lock_guard<std::mutex> lock(mtx);
    int removed = 0;
    auto it = list_store.find(key);
    if (it == list_store.end())
        return 0;
    auto& list = it->second;

    if (count == 0) {
        // Remove all occurances
        auto new_end = std::remove(list.begin(), list.end(), value);
        removed = std::distance(new_end, list.end());
        list.erase(new_end, list.end());
    } else if (count < 0) {
        // Remove from tail to head
        for (auto riter = list.rbegin(); riter != list.rend() && removed < (-count); ) {
            if (*riter == value) {
                auto fwdIterator = riter.base();
                --fwdIterator;
                fwdIterator = list.erase(fwdIterator);
                ++removed;
                riter = std::reverse_iterator<std::vector<std::string>::iterator>(fwdIterator);
            } else {
                ++riter;
            }
        }
    } else {
        // Remove from head to tail
        for (auto iter = list.begin(); iter != list.end() && removed < count; ) {
            if (*iter == value) {
                iter = list.erase(iter);
                ++removed;
            } else {
                ++iter;
            }
        }
    }
    return removed;
}


bool RedisDatabase::lindex(const std::string& key, int index, std::string& value) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = list_store.find(key);
    if (it == list_store.end())
        return false;
    
    const auto& list = it->second;
    if (index < 0)
        index = list.size() + index;
    if (index < 0 || index >= static_cast<int>(list.size())) 
        return false;
    
    value = list[index];

    return true;
}

bool RedisDatabase::lset(const std::string& key, int index, const std::string& value) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = list_store.find(key);
    if (it == list_store.end())
        return false;
    
    auto& list = it->second;
    if (index < 0)
        index = list.size() + index;
    if (index < 0 || index >= static_cast<int>(list.size())) 
        return false;
    
    list[index] = value;
    return true;
}