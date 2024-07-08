#include "client.hpp"
#include <iostream>
#include <stdexcept>
#include <atomic>

int main(int argc, char** argv)
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <src_mac> <dest_mac>" << std::endl;
        return 1;
    }

    std::string src_mac = argv[1];
    std::string dest_mac = argv[2];

    Client client(src_mac, dest_mac);

    std::string command;
    std::atomic<int> id_counter(1); // Initialize the ID counter

    while (true) {
        std::cout << "Enter command (store <message> / request <id> / exit): ";
        std::getline(std::cin, command);

        if (command == "exit") {
            break;
        }

        if (command.rfind("store ", 0) == 0) {
            std::string message = command.substr(6);
            int id = id_counter++;
            std::string packet_message = std::to_string(id) + ":" + message;
            client.sendEthernetPacket(packet_message, false);
            std::cout << "Stored message with ID: " << id << std::endl;
        } else if (command.rfind("request ", 0) == 0) {
            std::string id_str = command.substr(8);
            try {
                int id = std::stoi(id_str);
                client.sendEthernetPacket(std::to_string(id), true);
            } catch (const std::invalid_argument& e) {
                std::cerr << "Invalid ID: " << id_str << std::endl;
            } catch (const std::out_of_range& e) {
                std::cerr << "ID out of range: " << id_str << std::endl;
            }
        } else {
            std::cerr << "Invalid command" << std::endl;
        }
    }

    return 0;
}