# Building a Redis-like Server in C++

## Table of Contents

[toc]

## Day One: Setting Up the Core Server Loop

---

### Step 1: Set Up the Server Socket

We created a `RedisServer` class and initialized a TCP socket using `socket()`, then bound it to port 6379 using `bind()`, and started listening with `listen()`.

**Key Concepts:**
- Socket creation and setup
- Binding and listening for clients

---

### Step 2: Accept and Handle Connections

We added a `run()` method in `RedisServer` that accepts incoming client connections with `accept()` and reads requests using `recv()`.

---

### Step 3: Implement a Command Handler

A `RedisCommandHandler` class parses RESP-formatted input and handles commands like `PING`. It returns a valid Redis response.

---

### Step 4: Add Optional Persistence

We launched a detached thread that sleeps for 300 seconds and can later be used to persist the database.

---

### Step 5: Connect Everything in main()

The `main()` function now:
- Instantiates `RedisServer`
- Launches the persistence thread
- Calls `run()` to start the server loop

---

### Summary

We now have a Redis-like server that:
- Accepts client connections
- Parses RESP commands
- Has a placeholder for persistence

---

## Day Two: Handling Clients and Adding Persistence

---

### Step 1: Accept and Handle Multiple Clients

We updated the `run()` method in `RedisServer.cpp` to support multiple concurrent clients:

- Each client is handled in a separate thread stored in a thread-safe vector.
- Each client thread runs a `while (true)` loop that reads from the socket.
- Incoming data is processed using `RedisCommandHandler`.

---

### Step 2: RedisCommandHandler Integration

The server uses `RedisCommandHandler` to parse and respond to commands, keeping logic modular and testable.

---

### Step 3: Thread Management

Threads are tracked in a `std::vector<std::thread>`, and joined before the server exits to ensure clean shutdown.

---

### Step 4: Add RedisDatabase Singleton

We implemented `RedisDatabase` as a singleton with:

```cpp
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
```

It will store key-value data and supports persistence to/from disk.

---

### Step 5: Background Persistence Thread

A detached thread is started in `main.cpp` that:
- Calls `RedisDatabase::dump()` every 300 seconds
- Runs independently in the background

---

### Step 6: Graceful Shutdown via Signal Handling

Signal handling is added to `RedisServer` for clean shutdown:

```cpp
void signalHandler(int signum) {
    if (globalServer) {
        globalServer->shutdown();
    }
    exit(signum);
}

void RedisServer::setupSignalHandler() {
    signal(SIGINT, signalHandler);
}
```

This ensures the database is persisted before exit.

---

### Step 7: Updated Build System

We updated the `Makefile` to:
- Compile all `.cpp` files
- Include `-pthread` for thread support

---

By the end of Day Two, the server supports:
- Multi-client concurrency
- RESP command handling
- Periodic data persistence
- Graceful shutdown