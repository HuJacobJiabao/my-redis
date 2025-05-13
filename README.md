# Building a Redis-like Server in C++

## Table of Contents

- [Day One: Setting Up the Core Server Loop](#day-one-setting-up-the-core-server-loop)
  - [Step 1: Set Up the Server Socket](#step-1-set-up-the-server-socket)
  - [Step 2: Accept and Handle Connections](#step-2-accept-and-handle-connections)
  - [Step 3: Implement a Command Handler](#step-3-implement-a-command-handler)
  - [Step 4: Add Optional Persistence](#step-4-add-optional-persistence)
  - [Step 5: Connect Everything in main()](#step-5-connect-everything-in-main)
  - [Summary](#summary)
  - [Next Steps](#next-steps)

## Day One: Setting Up the Core Server Loop

This tutorial walks you through building a Redis-like server in C++ from scratch. You'll implement RESP command parsing, socket-based communication, and command handling — laying the foundation for a lightweight, extensible Redis clone.

---

##  Step 1: Set Up the Server Socket

###  What You’ll Do
- Create a `RedisServer` class.
- Initialize a TCP socket using `socket()`.
- Bind it to a port with `bind()`.
- Listen for incoming connections using `listen()`.

###  Key Concepts
- **Socket**: Endpoint for network communication.
- **Binding**: Associates socket with an IP and port.
- **Listening**: Puts the socket into a passive state to accept connections.

---

##  Step 2: Accept and Handle Connections

###  What You’ll Do
- Add a `run()` method to start the server loop.
- Use `accept()` to handle new client connections.
- Read and respond to incoming messages.

###  Key Concepts
- **Accepting clients**: Waits for and establishes a connection.
- **Request loop**: Continuously reads and writes to/from the socket.

---

##  Step 3: Implement a Command Handler

###  What You’ll Do
- Create a `RedisCommandHandler` class.
- Parse input using RESP (Redis Serialization Protocol).
- Handle basic commands like `PING`.

###  Key Concepts
- **RESP**: Text-based protocol used by Redis clients.
- **Command processing**: Map parsed commands to corresponding logic and responses.

---

##  Step 4: Add Optional Persistence

###  What You’ll Do
- Start a detached background thread.
- Periodically run a persistence task (e.g., every 300 seconds).

###  Key Concepts
- **Threading**: Run tasks in the background without blocking.
- **Persistence**: Optional, but critical for saving in-memory data.

---

##  Step 5: Connect Everything in `main()`

###  What You’ll Do
- Instantiate `RedisServer` with a port (default: 6379).
- Start optional persistence thread.
- Launch the main server loop via `run()`.

---

##  Summary

By the end of this tutorial, you will have built a simple Redis-like server in C++ that:
- Listens on a TCP socket.
- Handles basic RESP-formatted commands like `PING`.
- Responds to clients using the standard Redis protocol.

---

##  Next Steps

Once your basic server works, you can:
- Add support for commands like `ECHO`, `SET`, `GET`.
- Implement a key-value store (e.g., `std::unordered_map`).
- Improve performance with non-blocking I/O or multithreading.
- Add RDB or AOF persistence support like real Redis.