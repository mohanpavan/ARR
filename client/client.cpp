#include <cstring>
#include "client.hpp"

Client::Client(const std::string& src_mac, const std::string& dest_mac)
    : src_mac_(src_mac), dest_mac_(dest_mac) {
    interface_ = EthernetHandler::getInterfaceNameByMacAddress(src_mac_);
    if (interface_.empty()) {
        throw std::runtime_error("No interface found with the given MAC address");
    }
}

int Client::createRawSocket() {
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        throw std::runtime_error("Socket creation error");
    }
    return sockfd;
}

void Client::bindSocketToInterface(int sockfd) {
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interface_.c_str(), IFNAMSIZ - 1);
    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (void*)&ifr, sizeof(ifr)) < 0) {
        throw std::runtime_error("Bind to device failed");
    }
}

void Client::sendEthernetPacket(const std::string& message, bool is_request) {
    int sockfd = createRawSocket();
    bindSocketToInterface(sockfd);

    uint8_t action = is_request ? 0x80 : 0x00;
    std::string packet_message = std::string(1, action) + message;
    std::vector<uint8_t> packet = ethernet_handler_.assembleEthernetFrame(packet_message, src_mac_, dest_mac_, 0x0800);

    struct sockaddr_ll saddrll;
    memset(&saddrll, 0, sizeof(saddrll));
    saddrll.sll_family = AF_PACKET;
    saddrll.sll_ifindex = if_nametoindex(interface_.c_str());
    saddrll.sll_protocol = htons(ETH_P_ALL);

    if (sendto(sockfd, packet.data(), packet.size(), 0, (struct sockaddr*)&saddrll, sizeof(saddrll)) < 0) {
        std::cerr << "Send failed" << std::endl;
        close(sockfd);
        return;
    }

    if (is_request) {
        uint8_t buffer[ETH_FRAME_LEN];
        ssize_t num_bytes = recvfrom(sockfd, buffer, ETH_FRAME_LEN, 0, nullptr, nullptr);
        if (num_bytes > 0) {
            std::vector<uint8_t> response(buffer, buffer + num_bytes);
            std::string_view response_message = ethernet_handler_.getMessage(response);
            std::cout << "Response: " << response_message << std::endl;
        }
    }

    close(sockfd);
}