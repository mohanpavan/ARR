#pragma once
#include <cstdint>
#include <stddef.h>
#include <vector>
#include <string_view>

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
                                               int ethertype);

    /**
     * @brief Get the message contained in an ethernet packet.
     *
     * @param ethernet_packet The ethernet packet
     * @returns A read-only object to the message contained in the ethernet packet
     */
    std::string_view getMessage(std::vector<uint8_t>& ethernet_packet);

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