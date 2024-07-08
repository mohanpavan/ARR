#include "server.hpp"
#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <csignal>
#include <atomic>

std::atomic<bool> running(true);

void signalHandler(int signum) {
    running = false;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <buffer_capacity> <server_mac>" << std::endl;
        return 1;
    }

    uint32_t buffer_capacity = std::stoi(argv[1]);
    std::string server_mac = argv[2];
    Server server(buffer_capacity, server_mac);

    // Register signal handler for graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    // Start the server in a separate thread to handle incoming packets
    std::thread server_thread([&server]() {
        server.startServer();
    });

    // Listen for packet requests in the main thread
    server.listenForPackets();

    // Join the server thread before exiting
    if (server_thread.joinable()) {
        server_thread.join();
    }

    return 0;
}