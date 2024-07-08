#pragma once
#include "buffer.hpp"
#include "ethernet_handler.hpp"
#include <atomic>

extern std::atomic<bool> running;  // Added this line

/**
 * @brief The Server class
 * The server needs to provide two sockets for interacting with the clients:
 * A socket for incoming ethernet packets. Once a packet arrives, the server needs to extract the payload and store it
 * in a buffer. A socket for requests for a packet and sending the corresponding packet. A request containing an ID is
 * sent by a client and once the server receives it, it needs to send the corresponding packet stored in the buffer (if
 * it exists).
 */
class Server
{
public:
    Server(uint32_t capacity, const std::string& server_mac)
        : buffer_(capacity), server_mac_(server_mac)
    {
    }

    /**
     * @brief Listens to a socket for incoming ethernet packets. Once an ethernet packet arrives, the function needs to
     * verify that it's correct. If it is, then it needs to extract the payload and store it in the buffer. If it's not,
     * it needs to log it and abort.
     */
    void startServer();

    /**
     * @brief Listens to a socket for incoming requests for an ethernet packet. Each request needs to have an ID number
     * and once it's received, the function needs to retrieve from the buffer the corresponding message and send it back
     * to the client.
     */
    void listenForPackets(); 

private:
    int socket_;
    Buffer buffer_;
    EthernetHandler ethernet_handler_;
    std::string server_mac_;
};