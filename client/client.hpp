#pragma once

#include <iostream>
#include "ethernet_handler.hpp"
#include <string>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/ioctl.h>

class Client {
public:
    Client(const std::string& interface, const std::string& src_mac, const std::string& dest_mac);

    void sendEthernetPacket(const std::string& message, bool is_request);

private:
    std::string interface_;
    std::string src_mac_;
    std::string dest_mac_;
    EthernetHandler ethernet_handler_;

    int createRawSocket();
    void bindSocketToInterface(int sockfd);
};