# 351 TCP Echo Server

This is a multi-threaded TCP echo server that listens for incoming connections, receives messages from clients, and echoes the messages line by line. It supports handling multiple connections through threads and can accept new connections after one is terminated.

## Features

- **TCP Server**: Listens for client connections over TCP.
- **Echo Server**: For each received message, the server will send back the same message with the client's IP address and port.
- **Multithreaded**: Handles multiple client connections concurrently using threads.
- **Port Argument**: The server listens on a port specified by the user with the `-p` argument.
- **Graceful Shutdown**: The server can be shut down gracefully by sending a SIGINT (Ctrl+C).
- **Supports Telnet and Browser Testing**: Can be tested using Telnet or a web browser.

## Requirements

- **clang**: The C compiler used for building the server.
- **make**: The build automation tool used to manage the compilation process.
- **pthread**: Library required for multi-threading support.
- **netinet/in.h**, **arpa/inet.h**, and other standard socket libraries for networking functionality.


## How to Compile

1. Clone or download the repository to your local machine.
2. Ensure you have **clang** and **make** installed on your system.
3. Use the following `make` command to compile the server:

   ```bash
   make
