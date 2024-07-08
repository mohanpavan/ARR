#include "server.hpp"
#include "ethernet_handler.hpp"
#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <cstring>

void Server::startServer() {
    struct sockaddr_ll saddrll;
    struct ifreq ifr;

    // Get the interface name based on the MAC address
    std::string interface = EthernetHandler::getInterfaceNameByMacAddress(server_mac_);
    if (interface.empty()) {
        throw std::runtime_error("No interface found with the given MAC address");
    }

    // Create a raw socket
    if ((socket_ = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
        throw std::runtime_error("Socket creation failed");
    }

    // Bind to the interface
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ - 1);
    if (setsockopt(socket_, SOL_SOCKET, SO_BINDTODEVICE, (void*)&ifr, sizeof(ifr)) < 0) {
        perror("SO_BINDTODEVICE");
        close(socket_);
        throw std::runtime_error("Bind to device failed");
    }

    std::cout << "Listening for incoming Ethernet frames on interface " << interface << "..." << std::endl;

    // Listen for incoming packets
    listenForPackets();

    close(socket_);
}

void Server::listenForPackets() {
    uint8_t buffer[ETH_FRAME_LEN];
    struct sockaddr_ll saddrll;
    socklen_t saddrll_len = sizeof(saddrll);

    while (running) {
        // Receive Ethernet frame
        ssize_t num_bytes = recvfrom(socket_, buffer, ETH_FRAME_LEN, 0, (struct sockaddr*)&saddrll, &saddrll_len);
        if (num_bytes < 0) {
            perror("recvfrom failed");
            continue;
        }

        // Extract MAC addresses
        uint8_t* dest_mac = buffer;
        uint8_t* src_mac = buffer + 6;
        uint16_t eth_proto = ntohs(*(uint16_t*)(buffer + 12));

        std::vector<uint8_t> packet(buffer, buffer + num_bytes);
        std::string_view message = ethernet_handler_.getMessage(packet);

        if (!message.empty()) {
            uint8_t action = message[0] & 0x80;
            int id = message[0] & 0x7F;
            std::string payload(message.substr(1));

            if (action == 0) {
                buffer_.setElement(id, std::vector<uint8_t>(payload.begin(), payload.end()));
            } else {
                std::vector<uint8_t> response = buffer_.getElement(id);
                std::vector<uint8_t> response_packet = ethernet_handler_.assembleEthernetFrame(
                    std::string(response.begin(), response.end()),
                    server_mac_,
                    std::string(reinterpret_cast<char*>(src_mac), 6),
                    0x0800
                );
                sendto(socket_, response_packet.data(), response_packet.size(), 0, (struct sockaddr*)&saddrll, saddrll_len);
            }
        }
    }
}