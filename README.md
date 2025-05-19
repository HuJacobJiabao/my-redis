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