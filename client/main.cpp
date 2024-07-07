#include "client.hpp"

int main(int argc, char** argv)
{
    if (argc != 6) {
        std::cerr << "Usage: " << argv[0] << " <interface> <src_mac> <dest_mac> <message>" << std::endl;
        return 1;
    }

    std::string interface = argv[1];
    std::string src_mac = argv[2];
    std::string dest_mac = argv[3];
    std::string message = argv[4];

    Client client(interface, src_mac, dest_mac);

    // Send a packet to store a message
    client.sendEthernetPacket(message, false);

    // Send a packet to request the stored message
    client.sendEthernetPacket(message, true);

    return 0;
}