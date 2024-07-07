#include "server.hpp"
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

void Server::startServer() {
    int sockfd;
    struct sockaddr_ll saddrll;
    struct ifreq ifr;

    // Create a raw socket
    if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
        throw std::runtime_error("Socket creation failed");
    }

    // Get the index of the network interface
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1); // Replace "eth0" with your interface name
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0) {
        throw std::runtime_error("ioctl SIOCGIFINDEX failed");
    }

    // Bind the socket to the network interface
    memset(&saddrll, 0, sizeof(saddrll));
    saddrll.sll_family = AF_PACKET;
    saddrll.sll_ifindex = ifr.ifr_ifindex;
    saddrll.sll_protocol = htons(ETH_P_ALL);
    if (bind(sockfd, (struct sockaddr*)&saddrll, sizeof(saddrll)) < 0) {
        throw std::runtime_error("Bind failed");
    }

    // Listen for incoming packets
    listenForPackets(sockfd);

    close(sockfd);
}

void Server::listenForPackets(int sockfd) {
    uint8_t buffer[ETH_FRAME_LEN];
    struct sockaddr_ll saddrll;
    socklen_t saddrll_len = sizeof(saddrll);

    while (true) {
        ssize_t num_bytes = recvfrom(sockfd, buffer, ETH_FRAME_LEN, 0, (struct sockaddr*)&saddrll, &saddrll_len);
        if (num_bytes < 0) {
            perror("recvfrom failed");
            continue;
        }

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
                    "12:34:56:78:9A:BC", // Replace with server's MAC address
                    "DE:AD:BE:EF:00:01", // Replace with client's MAC address
                    0x0800
                );
                sendto(sockfd, response_packet.data(), response_packet.size(), 0, (struct sockaddr*)&saddrll, saddrll_len);
            }
        }
    }
}