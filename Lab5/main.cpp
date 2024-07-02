#include <SFML/Network.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>

struct tcpMessage {
    unsigned char nVersion;
    unsigned char nType;
    unsigned short nMsgLen;
    char chMsg[1000];
};

std::vector<sf::TcpSocket*> clients;
std::vector<std::pair<sf::IpAddress, unsigned short>> clientDetails;
std::mutex mutex;
unsigned char currentVersion = 102; // Default version

void handleClientMessages(sf::TcpSocket* client) {
    while (true) {
        tcpMessage receivedMsg;
        std::size_t received;
        if (client->receive(&receivedMsg, sizeof(receivedMsg), received) == sf::Socket::Done) {
            if (receivedMsg.nVersion != 102) {
                // Ignore messages with different versions
                continue;
            }

            if (receivedMsg.nType == 77) {
                std::lock_guard<std::mutex> lock(mutex);
                for (sf::TcpSocket* otherClient : clients) {
                    if (otherClient != client) {
                        otherClient->send(&receivedMsg, sizeof(receivedMsg));
                    }
                }
            } else if (receivedMsg.nType == 201) {
                std::string msg(receivedMsg.chMsg, receivedMsg.nMsgLen);
                std::reverse(msg.begin(), msg.end());
                client->send(&receivedMsg, sizeof(receivedMsg));
            }
        }
    }
}

void listenForClients(unsigned short port) {
    sf::TcpListener listener;
    if (listener.listen(port) != sf::Socket::Done) {
        return;
    }

    while (true) {
        sf::TcpSocket* client = new sf::TcpSocket;
        if (listener.accept(*client) == sf::Socket::Done) {
            {
                std::lock_guard<std::mutex> lock(mutex);
                clients.push_back(client);
                clientDetails.push_back({client->getRemoteAddress(), client->getRemotePort()});
            }

            std::thread clientThread(handleClientMessages, client);
            clientThread.detach();
        }
    }
}

void printLastMessage() {
    // To be implemented: print the last received message
    // You'd need a variable to store the last received message
}

void printConnectedClients() {
    std::lock_guard<std::mutex> lock(mutex);
    std::cout << "Number of Clients: " << clients.size() << std::endl;
    for (const auto& client : clientDetails) {
        std::cout << "IP Address: " << client.first << " | Port: " << client.second << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: ./server <IP Address> <port>\n";
        return 1;
    }

    sf::IpAddress ipAddress(argv[1]);
    unsigned short port = std::stoi(argv[2]);

    std::thread clientListener(listenForClients, port);
    clientListener.detach();

    // Main application loop to handle user input
    while (true) {
        std::string command;
        std::cout << "Please enter command: ";
        std::cin >> command;

        if (command == "msg") {
            printLastMessage();
        } else if (command == "clients") {
            printConnectedClients();
        } else if (command == "exit") {
            for (sf::TcpSocket* client : clients) {
                client->disconnect();
                delete client;
            }
            return 0;
        }

        // Additional logic to handle other user commands
        // ...

        // For receiving messages, consider another thread to handle incoming server messages.
    }

    return 0;
}