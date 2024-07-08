#pragma once
#include <netdb.h>  // Add this line at the top of the file
#include <cstdint>
#include <stddef.h>
#include <vector>
#include <string>
#include <string_view>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <cstring>
#include <unistd.h>

/**
 * @brief The EthernetHandler class
 * The ethernet handler needs to provide two methods:
 * One to construct an ethernet packet based on a message
 * One to retrieve the message from an ethernet packet
 * Each ethernet packet needs to have the layout of a proper ethernet packet. You are free to choose the source and
 * destination MAC addresses. The payload of each packet should be a string with the following format: "\${ID}:
 * ${Message}", where `ID` is a non-negative integer and `Message` is the actual message.
 */
class EthernetHandler
{

public:
    // | Destination MAC | Source MAC    | Ethertype | Payload                | FCS       |
    // |-----------------|---------------|-----------|------------------------|-----------|
    // | DE AD BE EF 00 01 | 12 34 56 78 9A BC | 08 00    | 31 3A 48 65 6C 6C 6F 2C 20 57 6F 72 6C 64 21 | 12 34 56 78 |
    /**
     * @brief Assembles a complete ethernet frame in frame_buffer bytes.
     *
     * @param message Message that will be contained in the ethernet frame
     * @param source_mac_address Source of the ethernet packet
     * @param destination_mac_address destination of the ethernet packet
     * @param ethertype Type of the ethernet packet
     * @returns the final ethernet packet
     */
    std::vector<uint8_t> assembleEthernetFrame(std::string_view message,
                                               std::string_view source_mac_address,
                                               std::string_view destination_mac_address,
                                               int ethertype)
    {
        std::vector<uint8_t> frame;
        frame.reserve(header_size + message.size() + fcs_size);

        // Add destination MAC address
        for (size_t i = 0; i < mac_address_size; ++i) {
            frame.push_back(static_cast<uint8_t>(std::stoi(std::string(destination_mac_address.substr(i * 2, 2)), nullptr, 16)));
        }

        // Add source MAC address
        for (size_t i = 0; i < mac_address_size; ++i) {
            frame.push_back(static_cast<uint8_t>(std::stoi(std::string(source_mac_address.substr(i * 2, 2)), nullptr, 16)));
        }

        // Add Ethertype
        frame.push_back(static_cast<uint8_t>((ethertype >> 8) & 0xFF));
        frame.push_back(static_cast<uint8_t>(ethertype & 0xFF));

        // Add payload (message is already in the correct format)
        frame.insert(frame.end(), message.begin(), message.end());

        // Compute and add FCS
        uint32_t crc = computeCrc32(frame);
        frame.push_back(static_cast<uint8_t>((crc >> 24) & 0xFF));
        frame.push_back(static_cast<uint8_t>((crc >> 16) & 0xFF));
        frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
        frame.push_back(static_cast<uint8_t>(crc & 0xFF));

        return frame;
    }

    /**
     * @brief Get the message contained in an ethernet packet.
     *
     * @param ethernet_packet The ethernet packet
     * @returns A read-only object to the message contained in the ethernet packet
     */
    std::string_view getMessage(std::vector<uint8_t>& ethernet_packet)
    {
        if (ethernet_packet.size() < header_size + fcs_size) {
            return "";
        }

        // Verify FCS
        if (!verifyEthernetFrameFcs(ethernet_packet)) {
            return "";
        }

        // Extract payload
        size_t payload_start = header_size;
        size_t payload_end = ethernet_packet.size() - fcs_size;
        std::string_view payload(reinterpret_cast<char*>(&ethernet_packet[payload_start]), payload_end - payload_start);

        return payload;
    }

    static std::string getInterfaceNameByMacAddress(const std::string& mac_address) {
        struct ifaddrs *ifaddr, *ifa;
        int family, s;
        char host[NI_MAXHOST];

        if (getifaddrs(&ifaddr) == -1) {
            perror("getifaddrs");
            return "";
        }

        for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == nullptr)
                continue;

            family = ifa->ifa_addr->sa_family;

            if (family == AF_PACKET) {
                struct ifreq ifr;
                int fd = socket(AF_INET, SOCK_DGRAM, 0);
                if (fd == -1) {
                    perror("socket");
                    continue;
                }

                strncpy(ifr.ifr_name, ifa->ifa_name, IFNAMSIZ - 1);
                if (ioctl(fd, SIOCGIFHWADDR, &ifr) == -1) {
                    perror("ioctl");
                    close(fd);
                    continue;
                }

                close(fd);

                uint8_t* mac = reinterpret_cast<uint8_t*>(ifr.ifr_hwaddr.sa_data);
                char mac_str[18];
                snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
                         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

                if (mac_address == mac_str) {
                    freeifaddrs(ifaddr);
                    return ifa->ifa_name;
                }
            }
        }

        freeifaddrs(ifaddr);
        return "";
    }

private:
    static constexpr uint8_t mac_address_size{6};
    static constexpr uint8_t header_size{14};
    static constexpr uint8_t fcs_size{4};

private:
    /**
     * @brief Computes a crc32.
     *
     * @param data The ethernet packet without the FCS
     * @returns The CRC
     */
    uint32_t computeCrc32(const std::vector<uint8_t>& data)
    {
        const static int32_t polynomial   = 0xEDB88320;
        const static uint32_t initializer = 0xFFFFFFFFu;

        static uint32_t CRCTable[256];
        static int table_created = 0;
        if (!table_created) {
            for (int i = 0; i < 256; ++i) {
                uint32_t remainder = i;
                for (int j = 0; j < 8; ++j) {
                    if (remainder & 1) {
                        remainder >>= 1;
                        remainder ^= polynomial;
                    }
                    else {
                        remainder >>= 1;
                    }
                }
                CRCTable[i] = remainder;
            }
            table_created = 1;
        }

        uint32_t crc = initializer;

        for (size_t i = 0; i < data.size(); ++i) {
            const uint32_t lookupIndex = (crc ^ data[i]) & 0xFF;
            crc                        = (crc >> 8) ^ CRCTable[lookupIndex];
        }

        crc ^= 0xFFFFFFFFu;
        return crc;
    }

    /**
     * @brief Verifies if the ethernet frame's checksum is correct.
     *
     * @param frame_buffer Contains a complete ethernet frame including FCS
     * @returns True if FCS is incorrect, false otherwise
     */
    bool verifyEthernetFrameFcs(const std::vector<uint8_t>& frame_buffer)
    {
        uint32_t check = computeCrc32(frame_buffer);
        return check == 0x2144DF1C;
    }
};