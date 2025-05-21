# Redis-like Server in C++

This project is a Redis-compatible, in-memory key-value database server implemented in modern C++. It supports core Redis data types (strings, lists, hashes), RESP protocol parsing, concurrent client handling, key expiration, and periodic persistence to disk. The server is designed for educational purposes and demonstrates scalable network programming, data structure management, and basic database features in C++.

**Tested Environment:**
- WSL: Ubuntu 22.04.5 LTS
- C++: GCC/G++ 11.4.0 (C++17 standard)

## Technical Details
- Modern C++ (C++17): RAII, smart pointers, STL containers (unordered_map, vector, etc.)
- Linux socket programming: TCP server, client handling, multi-threading
- Thread safety: std::mutex, lock_guard
- RESP protocol parsing and serialization
- In-memory data structures: string, list, hash
- Key expiration and time management (std::chrono)
- Data persistence: file I/O for dump/load
- Modular code organization and design patterns (Singleton)

For a detailed, step-by-step tutorial and development log, see [day_by_day.md](./day_by_day.md).

---

## Features
- RESP protocol support (compatible with `redis-cli` and other clients)
- Key-value, list, and hash data structures
- Expiration for all key types (`EXPIRE` command)
- Periodic persistence to disk (except expiration data)
- Multi-threaded, concurrent client handling
- Modular, maintainable C++ codebase

## Supported Commands

### Key-Value Commands
| Command | Description |
|---------|-------------|
| `SET`, `GET` | Set or get a string value by key |
| `DEL`, `UNLINK` | Delete or asynchronously delete a key |
| `EXPIRE` | Set a timeout on a key |
| `RENAME` | Rename a key |
| `TYPE` | Get the type of value stored at a key |
| `KEYS` | List all keys |
| `FLUSHALL` | Remove all keys |
| `PING`, `ECHO` | Health check and echo message |

### List Commands
| Command | Description |
|---------|-------------|
| `LLEN` | Get the length of a list |
| `LPUSH`, `RPUSH` | Push one or more values to the head/tail of a list |
| `LPOP`, `RPOP` | Pop a value from the head/tail of a list |
| `LREM` | Remove elements from a list |
| `LINDEX` | Get an element by index |
| `LSET` | Set the value of an element by index |

### Hash Commands
| Command | Description |
|---------|-------------|
| `HSET`, `HMSET` | Set one or more fields in a hash |
| `HGET` | Get the value of a field in a hash |
| `HEXISTS` | Check if a field exists in a hash |
| `HDEL` | Delete a field from a hash |
| `HGETALL` | Get all fields and values in a hash |
| `HKEYS` | Get all field names in a hash |
| `HVALS` | Get all values in a hash |
| `HLEN` | Get the number of fields in a hash |

## How to Build and Run
1. Clone the repository and navigate to the project directory.
2. Build the project:
   ```sh
   make
   ```
3. Run the server:
   ```sh
   ./my_redis_server
   ```
4. (Optional) Use `redis-cli` or your own client to connect to `localhost:6379` and issue commands.

---

## License
This project is for educational purposes and personal practice only, and is not affiliated with Redis or Redis Labs.
