# Building a Redis-like Server in C++

This project is a step-by-step tutorial for building a Redis-like in-memory key-value database server in C++. It covers the core concepts of network programming, command parsing, concurrency, and persistence. By following each day's instructions, you will implement a server that can accept client connections, parse and respond to Redis protocol commands, handle multiple clients concurrently, and lay the groundwork for data persistence and advanced features. The project is structured to help you learn and apply modern C++ techniques for building scalable networked applications.

[toc]

## Day 1: Setting Up the Core Server Loop

### Introduction
On Day 1, you will build the foundation of your Redis-like server. You will set up a TCP server socket, accept client connections, parse basic RESP commands, and prepare for future persistence. By the end of today, your server will be able to accept a single client and respond to simple commands like PING.

### What You Should Do
#### 1. Set Up the Server Socket
- Create a `RedisServer` class.
- In the constructor, use `socket()` to create a TCP socket, `bind()` it to port 6379, and call `listen()` to start listening for connections.
- Handle errors if the port is in use or if socket creation fails.
- May need to run `sudo service redis-server stop` to close the default redis-server.

#### 2. Accept and Handle Connections
- Add a `run()` method to `RedisServer`.
- In a loop, call `accept()` to accept incoming client connections.
- Use `recv()` to read requests from each client.
- For now, handle one client at a time in a blocking loop.

#### 3. Implement a Command Handler
- Create a `RedisCommandHandler` class.
- Implement a function to parse RESP input and extract command tokens.
- Add logic to handle the `PING` command and return a valid Redis response (e.g., `+PONG\r\n`).

#### 4. Add Optional Persistence
- In your `main()` function, launch a detached thread that sleeps for 300 seconds (or any interval you choose).
- This thread will later be used to persist the database to disk. For now, the thread can simply sleep in a loop.

#### 5. Connect Everything in main()
- In your `main()` function, instantiate the `RedisServer` class, launch the persistence thread, and call `run()` to start the server loop.

---

## Day 2: Handling Clients and Adding Persistence

### Introduction
On Day 2, you will extend your server to handle multiple clients concurrently, integrate the command handler for all client requests, manage threads, and lay the groundwork for persistence and clean shutdowns. By the end of today, your server will be able to serve multiple clients and be ready for data persistence.

### What You Should Do
#### 1. Accept and Handle Multiple Clients
- Update the `run()` method in `RedisServer.cpp` to support multiple concurrent clients.
- For each new client connection, spawn a new thread to handle that client.
- Store each thread in a `std::vector<std::thread>`.
- Each client thread should run a loop that reads from the socket and processes commands.
- Join all threads before the server exits to ensure a clean shutdown.

#### 2. RedisCommandHandler Integration
- In each client thread, pass received data to `RedisCommandHandler::handleCommand()` and send the response back to the client.

#### 3. Thread Management
- After the main server loop ends, iterate over the thread vector and call `join()` on each thread if it is joinable.

#### 4. Add RedisDatabase Singleton
- Implement a `RedisDatabase` class as a singleton to store key-value data and support persistence to/from disk.
- Define the singleton pattern in `RedisDatabase.h`.
- Add method prototypes for persistence (e.g., `dump()` and `load()`).

#### 5. Background Persistence Thread
- Start a detached thread in `main.cpp` that calls `RedisDatabase::dump()` every 300 seconds (or your chosen interval).
- In the background thread, periodically call the `dump()` method to save the database to disk.

#### 6. Graceful Shutdown via Signal Handling
- Register a signal handler that calls `RedisServer::shutdown()` and exits.
- Ensure the database is persisted before exit.

#### 7. Updated Build System
- Update your `Makefile` to compile all `.cpp` files and include `-pthread` for thread support.

---

## Day 3: Implementing the Database Layer

### Command Handler for Key-Value Operations

In this step, we extend our `RedisCommandHandler` so it can parse and handle a variety of key-value commands such as `SET`, `GET`, `DEL`, `UNLINK`, `EXPIRE`, `RENAME`, `TYPE`, `KEYS`, and `FLUSHALL`. The handler parses RESP input, extracts the command and arguments, and calls the corresponding method in the `RedisDatabase` singleton.

However, at this stage, the actual logic for these database operations is not implemented. Instead, we define the function prototypes in `RedisDatabase.h` and provide empty or placeholder implementations in `RedisDatabase.cpp`. This sets up the structure for the database layer, making it easy to add the real logic in the next steps.

### What You Should Do
#### 1. Update the Command Handler:
- In `RedisCommandHandler.cpp`, add logic to parse and dispatch commands to the database layer. For each supported command, call the corresponding method in `RedisDatabase`.
- Example: For `SET`, call `db.set(key, value)`. For `GET`, call `db.get(key, value)`.

#### 2. Define Database Function Prototypes:
- In `RedisDatabase.h`, declare all the methods you plan to support, such as:
  ```cpp
  bool set(const std::string& key, const std::string& value);
  bool get(const std::string& key, std::string& value);
  bool del(const std::string& key);
  bool exists(const std::string& key);
  std::vector<std::string> keys();
  std::string type(const std::string& key);
  bool expire(const std::string& key, const std::string& seconds);
  bool rename(const std::string& oldKey, const std::string& newKey);
  bool flushAll();
  ```
- These should match the calls made in your command handler.

#### 3. Stub Out the Implementations:
- In `RedisDatabase.cpp`, provide empty or placeholder implementations for each method. For now, these can simply return `false`, `true`, or empty values as appropriate.
- Example:
  ```cpp
  bool RedisDatabase::set(const std::string& key, const std::string& value) {
      // TODO: Implement this
      return false;
  }
  ```

#### 4. Build and Test the Skeleton:
- At this point, your server will parse commands and call the right database methods, but the database will not actually store or retrieve any data. This is a good time to check that your command handler and function signatures are correct before moving on to real logic.

---

### Supported Commands

This server currently supports the following Redis-like commands:

| Command                  | Function                                               | Example Request (RESP)                                 | Example Response                  |
|--------------------------|--------------------------------------------------------|--------------------------------------------------------|------------------------------------|
| `PING`                   | Health check                                          | `*1\r\n$4\r\nPING\r\n`                                   | `+PONG\r\n`                        |
| `ECHO <message>`         | Echoes back the message                               | `*2\r\n$4\r\nECHO\r\n$5\r\nhello\r\n`                  | `+hello\r\n`                       |
| `SET <key> <value>`      | Sets a key to a value                                 | `*3\r\n$3\r\nSET\r\n$3\r\nfoo\r\n$3\r\nbar\r\n`         | `+OK\r\n`                          |
| `GET <key>`              | Gets the value of a key                               | `*2\r\n$3\r\nGET\r\n$3\r\nfoo\r\n`                     | `$3\r\nbar\r\n` or `$-1\r\n`      |
| `DEL <key>`              | Deletes a single key                                  | `*2\r\n$3\r\nDEL\r\n$3\r\nfoo\r\n`                      | `:1\r\n` (1 if key deleted, 0 else)   |
| `UNLINK <key>`           | Asynchronously deletes a single key                   | `*2\r\n$6\r\nUNLINK\r\n$3\r\nfoo\r\n`                   | `:1\r\n` (1 if key unlinked, 0 else) |
| `FLUSHALL`               | Removes all keys from all databases                   | `*1\r\n$8\r\nFLUSHALL\r\n`                                | `+OK\r\n`                          |
| `TYPE <key>`             | Returns the type of the value stored at key           | `*2\r\n$4\r\nTYPE\r\n$3\r\nfoo\r\n`                      | `+string\r\n` or `+none\r\n`       |
| `KEYS <pattern>`         | Returns all keys (pattern matching not supported yet) | `*2\r\n$4\r\nKEYS\r\n$1\r\n*\r\n`                         | `*1\r\n$3\r\nfoo\r\n` (array)      |
| `EXPIRE <key> <seconds>` | Sets a timeout on a key                               | `*3\r\n$6\r\nEXPIRE\r\n$3\r\nfoo\r\n$2\r\n10\r\n`         | `:1\r\n` (1 if timeout set, 0 else) |
| `RENAME <key> <newkey>`  | Renames a key                                         | `*3\r\n$6\r\nRENAME\r\n$3\r\nfoo\r\n$6\r\nfoobar\r\n`      | `+OK\r\n` or error                 |
| Unknown command          | Any unsupported command                               | `*1\r\n$7\r\nUNKNOWN\r\n`                                  | `-Error: Unknown command\r\n`      |

### Command Details

- **PING**: Returns `+PONG\r\n` to confirm the server is alive.
- **ECHO <message>**: Returns the message sent. If no message is provided, returns `-Error: ECHO command requires a message\r\n`.
- **SET <key> <value>**: Stores the value under the key. Returns `+OK\r\n`.
- **GET <key>**: Returns the value for the key if it exists, or `$-1\r\n` if not found.
- **DEL <key>**: Deletes a single key. Returns `:1\r\n` if the key was deleted, or `:0\r\n` if the key did not exist.
- **UNLINK <key>**: Asynchronously deletes a single key. Returns `:1\r\n` if the key was unlinked, or `:0\r\n` if the key did not exist.
- **FLUSHALL**: Removes all keys from all databases. Returns `+OK\r\n`.
- **TYPE <key>**: Returns the type of the value stored at key (`+string\r\n`, `+none\r\n`, etc.).
- **KEYS <pattern>**: Returns all keys as an array reply. Pattern matching is not supported in this implementation; all keys are returned regardless of the pattern provided.
- **EXPIRE <key> <seconds>**: Sets a timeout on a key. Returns `:1\r\n` if the timeout was set, or `:0\r\n` if the key does not exist.
- **RENAME <key> <newkey>**: Renames a key. Returns `+OK\r\n` if successful, or an error if the source key does not exist.
- **Unknown/Unsupported**: Returns `-Error: Unknown command\r\n`.

All commands use the [RESP (REdis Serialization Protocol)](https://redis.io/docs/reference/protocol-spec/) format.

---

## Day 4: Implementing Key-Value and List Operations

### Introduction
On Day 4, you will implement the actual logic for key-value operations in the `RedisDatabase` class and lay the groundwork for list operations. This means your server will now be able to store, retrieve, and delete string values, as well as manage lists with commands like `LLEN`, `LPUSH`, `RPUSH`, `LPOP`, `RPOP`, `LREM`, `LINDEX`, and `LSET`. You will also add the function prototypes for list operations in the header, provide their implementations in the source file, and update the command handler to dispatch these commands.

### What You Should Do
#### 1. Implement Key-Value Operations in RedisDatabase
- In `RedisDatabase.cpp`, implement the following methods:
  - `void set(const std::string& key, const std::string& value);`
  - `bool get(const std::string& key, std::string& value);`
  - `bool del(const std::string& key);`
  - `bool exists(const std::string& key);`
  - `std::vector<std::string> keys();`
  - `std::string type(const std::string& key);`
  - `bool expire(const std::string& key, int seconds);`
  - `bool rename(const std::string& oldKey, const std::string& newKey);`
  - `bool flushAll();`
- These methods should now actually store, retrieve, and manage data in the in-memory database.

#### 2. Add List Operation Prototypes
- In `RedisDatabase.h`, add the following method prototypes for list operations:
  ```cpp
  ssize_t llen(const std::string& key);
  void lpush(const std::string& key, const std::string& value);
  void rpush(const std::string& key, const std::string& value);
  bool lpop(const std::string& key, std::string& value);
  bool rpop(const std::string& key, std::string& value);
  int lrem(const std::string& key, int count, const std::string& value);
  bool lindex(const std::string& key, int index, std::string& value);
  bool lset(const std::string& key, int index, const std::string& value);
  ```

#### 3. Implement List Operations in RedisDatabase
- In `RedisDatabase.cpp`, implement the above list methods. For example:
  - `llen`: Return the length of the list at the given key.
  - `lpush`: Insert a value at the head of the list.
  - `rpush`: Insert a value at the tail of the list.
  - `lpop`: Remove and return the first element of the list.
  - `rpop`: Remove and return the last element of the list.
  - `lrem`: Remove elements equal to value as described in the Redis docs.
  - `lindex`: Get the element at a specific index.
  - `lset`: Set the element at a specific index.

#### 4. Update the Command Handler
- In `RedisCommandHandler.cpp`, add `else if` branches for each list command:
  - `LLEN`, `LPUSH`, `RPUSH`, `LPOP`, `RPOP`, `LREM`, `LINDEX`, `LSET`
- Each branch should call the corresponding handler function, which in turn calls the appropriate method in `RedisDatabase`.

#### 5. Build and Test
- Rebuild your project and test the key-value and list commands using your client or `redis-cli`.
- Verify that data is stored, retrieved, and manipulated as expected.

---

### Supported List Commands

| Command                        | Function                                         | Example Request (RESP)                                   | Example Response                  |
|--------------------------------|--------------------------------------------------|----------------------------------------------------------|------------------------------------|
| `LLEN <key>`                   | Returns the length of the list                   | `*2\r\n$4\r\nLLEN\r\n$3\r\nmylist\r\n`                | `:2\r\n` (list length)             |
| `LPUSH <key> <value> [value ...]` | Inserts one or more values at the head of the list | `*5\r\n$5\r\nLPUSH\r\n$3\r\nmylist\r\n$3\r\nfoo\r\n$3\r\nbar\r\n$3\r\nbaz\r\n` | `:3\r\n` (new list length)         |
| `RPUSH <key> <value> [value ...]` | Inserts one or more values at the tail of the list | `*5\r\n$5\r\nRPUSH\r\n$3\r\nmylist\r\n$3\r\nfoo\r\n$3\r\nbar\r\n$3\r\nbaz\r\n` | `:3\r\n` (new list length)         |
| `LPOP <key>`                   | Removes and returns the first element            | `*2\r\n$4\r\nLPOP\r\n$3\r\nmylist\r\n`                | `$3\r\nfoo\r\n` or `$-1\r\n`      |
| `RPOP <key>`                   | Removes and returns the last element             | `*2\r\n$4\r\nRPOP\r\n$3\r\nmylist\r\n`                | `$3\r\nbar\r\n` or `$-1\r\n`      |
| `LREM <key> <count> <value>`   | Removes elements equal to value                  | `*4\r\n$4\r\nLREM\r\n$3\r\nmylist\r\n$1\r\n0\r\n$3\r\nfoo\r\n` | `:1\r\n` (number removed)          |
| `LINDEX <key> <index>`         | Gets the element at the specified index          | `*3\r\n$6\r\nLINDEX\r\n$3\r\nmylist\r\n$1\r\n0\r\n`   | `$3\r\nfoo\r\n` or `$-1\r\n`      |
| `LSET <key> <index> <value>`   | Sets the element at the specified index          | `*4\r\n$4\r\nLSET\r\n$3\r\nmylist\r\n$1\r\n0\r\n$3\r\nbaz\r\n` | `+OK\r\n` or error                 |

---

## Day 5: Implementing Hash Operations

### Introduction
On Day 5, you will implement hash operations in the `RedisDatabase` class and extend the command handler to support hash commands. Your server will be able to handle hashes, which are collections of key-value pairs, and respond to commands like `HSET`, `HGET`, `HDEL`, `HKEYS`, `HVALS`, and `HLEN`. You will also add support for the `HMSET` command to set multiple fields in a hash.

### What You Should Do
#### 1. Add Hash Data Structure
- In `RedisDatabase.h`, add a new data member:
  ```cpp
  std::unordered_map<std::string, std::unordered_map<std::string, std::string>> hashTable;
  ```
- This will store hashes with the key as the outer map key and field-value pairs as the inner map.

#### 2. Implement Hash Operations
- In `RedisDatabase.cpp`, implement the following methods:
  - `void hset(const std::string& key, const std::string& field, const std::string& value);`
  - `bool hget(const std::string& key, const std::string& field, std::string& value);`
  - `bool hdel(const std::string& key, const std::string& field);`
  - `bool hexists(const std::string& key, const std::string& field);`
  - `std::vector<std::string> hkeys(const std::string& key);`
  - `std::vector<std::string> hvals(const std::string& key);`
  - `size_t hlen(const std::string& key);`
  - `void hmset(const std::string& key, const std::unordered_map<std::string, std::string>& fields);`

#### 3. Update the Command Handler
- In `RedisCommandHandler.cpp`, add `else if` branches for each hash command:
  - `HSET`, `HGET`, `HDEL`, `HEXISTS`, `HKEYS`, `HVALS`, `HLEN`, `HMSET`
- Each branch should call the corresponding handler function, which in turn calls the appropriate method in `RedisDatabase`.

#### 4. Build and Test
- Rebuild your project and test the hash commands using your client or `redis-cli`.
- Verify that hash data is stored, retrieved, and manipulated as expected.

---

### Supported Hash Commands

| Command                                 | Function                                         | Example Request (RESP)                                   | Example Response                  |
|------------------------------------------|--------------------------------------------------|----------------------------------------------------------|------------------------------------|
| `HSET <key> <field> <value>`             | Sets field in the hash stored at key              | `*4\r\n$4\r\nHSET\r\n$6\r\nuser:1\r\n$4\r\nname\r\n$3\r\nBob\r\n` | `:1\r\n` (1 if new field, 0 if updated) |
| `HGET <key> <field>`                     | Gets the value of a field in a hash               | `*3\r\n$4\r\nHGET\r\n$6\r\nuser:1\r\n$4\r\nname\r\n` | `$3\r\nBob\r\n` or `$-1\r\n`      |
| `HEXISTS <key> <field>`                  | Checks if a field exists in a hash                | `*3\r\n$7\r\nHEXISTS\r\n$6\r\nuser:1\r\n$4\r\nname\r\n` | `:1\r\n` (1 if exists, 0 else)      |
| `HDEL <key> <field>`                     | Deletes a field from a hash                       | `*3\r\n$4\r\nHDEL\r\n$6\r\nuser:1\r\n$4\r\nname\r\n` | `:1\r\n` (1 if deleted, 0 else)     |
| `HGETALL <key>`                          | Gets all fields and values in a hash              | `*2\r\n$4\r\nHGETALL\r\n$6\r\nuser:1\r\n` | `*4\r\n$4\r\nname\r\n$3\r\nBob\r\n$3\r\nage\r\n$2\r\n30\r\n` |
| `HKEYS <key>`                            | Gets all field names in a hash                    | `*2\r\n$5\r\nHKEYS\r\n$6\r\nuser:1\r\n` | `*2\r\n$4\r\nname\r\n$3\r\nage\r\n` |
| `HVALS <key>`                            | Gets all values in a hash                         | `*2\r\n$5\r\nHVALS\r\n$6\r\nuser:1\r\n` | `*2\r\n$3\r\nBob\r\n$2\r\n30\r\n` |
| `HLEN <key>`                             | Gets the number of fields in a hash               | `*2\r\n$4\r\nHLEN\r\n$6\r\nuser:1\r\n` | `:2\r\n` (number of fields)         |
| `HMSET <key> <field> <value> [field value ...]` | Sets multiple fields in a hash           | `*6\r\n$5\r\nHMSET\r\n$6\r\nuser:1\r\n$4\r\nname\r\n$3\r\nBob\r\n$3\r\nage\r\n$2\r\n30\r\n` | `:2\r\n` (number of fields updated) |

---

## Day 6: Expiration and Persistence

### Introduction
On Day 6, you will implement expiration for keys and fields, ensuring they are automatically removed after a set time. You will also finalize the persistence mechanism, allowing the database to be saved to and loaded from disk. This includes persisting key-value pairs, lists, and hashes, except for expiration data. By the end of today, your server will have a fully functional persistence layer and support for expirable keys and fields.

### What You Should Do
#### 1. Expiration Data Structure
- In `RedisDatabase.h`, add a new data member:
  ```cpp
  std::unordered_map<std::string, std::chrono::steady_clock::time_point> expirations;
  ```
- This will store expiration times for keys.

#### 2. Update Set and Expire Methods
- In `RedisDatabase.cpp`, update the `set` and `expire` methods to handle expiration:
  - `bool RedisDatabase::set(const std::string& key, const std::string& value, int seconds)`: Set the value and expiration time.
  - `bool RedisDatabase::expire(const std::string& key, int seconds)`: Update the expiration time.

#### 3. Implement Expiration Check
- In `RedisDatabase.cpp`, implement a method to check and remove expired keys:
  - `void RedisDatabase::removeExpired()`: Remove keys that have expired.

#### 4. Update Dump and Load Methods
- In `RedisDatabase.cpp`, update the `dump` and `load` methods to handle expiration:
  - `void RedisDatabase::dump()`: Persist all data to disk, including expiration times.
  - `void RedisDatabase::load()`: Load data from disk and set expiration times.

#### 5. Signal Handling for Persistence
- In `main.cpp`, update the signal handler to call `RedisDatabase::dump()` before exiting.

#### 6. Build and Test
- Rebuild your project and test the expiration and persistence features.
- Verify that data is persisted across restarts and that expired data is not returned by the server.

---

### Additional Features

- **Expiration**: The `EXPIRE` command sets a timeout on any key (string, list, or hash). Expired keys are automatically removed on access.
- **Persistence**: The database is periodically saved to disk (except for expiration data). On restart, the database is loaded from the last dump.
- **LPUSH/RPUSH Multi-Value**: Both `LPUSH` and `RPUSH` support inserting multiple values in a single command, matching Redis semantics.

---