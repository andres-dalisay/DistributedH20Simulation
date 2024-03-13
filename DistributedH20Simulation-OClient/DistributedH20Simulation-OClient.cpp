// client.cpp
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <format>
#include <sstream>
#include <log.hpp>

#pragma comment(lib, "ws2_32.lib")

#define MASTER_SERVER_IP "127.0.0.1"
#define O_LIMIT 1048576

int main() {
    char oInput[100];


    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error initializing Winsock" << std::endl;
        return 3;
    }

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Error creating socket" << std::endl;
        WSACleanup();
        return 3;
    }

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(5001);
    inet_pton(AF_INET, MASTER_SERVER_IP, &server_address.sin_addr);

    if (connect(server_socket, reinterpret_cast<SOCKADDR*>(&server_address), sizeof(server_address)) == SOCKET_ERROR) {
        std::cerr << "Error connecting to server" << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 3;
    }

    int o_int;
    timestamp ts;

    while (true) {
        std::cout << "Enter number of oxygen atoms (LIMIT = " << O_LIMIT << "): ";
        std::cin.getline(oInput, sizeof(oInput));


        try {

            o_int = std::stoi(oInput);
            if (o_int < 0 || o_int > O_LIMIT) {
                std::cerr << "Error: Invalid input. ";
                if (o_int < 0) {
                    std::cerr << "Input must be positive. ";
                }
                if (o_int > O_LIMIT) {
                    std::cerr << "Input must be less than 1048576. ";
                }
                std::cerr << "Please try again." << std::endl;
            }
            else {
                break;
            }
        }
        catch (const std::invalid_argument& e) {
            std::cerr << "Error: Invalid input. Please enter numeric values." << std::endl;
        }
        catch (const std::out_of_range& e) {
            std::cerr << "Error: Input out of range." << std::endl;
        }
    }

    freopen("oxygen_log.txt", "w", stdout);
    for (int i = 1; i <= o_int; i++) {
        std::string logString = "O" + std::string(std::to_string(i)) + ", request\n";
        std::printf("%s", logString.c_str());

        // Send the serialized log data to the server
        int bytesSent = send(server_socket, logString.c_str(), logString.size(), 0);
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Failed to send data.\n";
            closesocket(server_socket);
            WSACleanup();
            return 1;
        }

    }
    fclose(stdout);
    
    const int bufferSize = 1024;
    char buffer[bufferSize];
    int bytesReceived;

    //do {
    //    bytesReceived = recv(server_socket, buffer, bufferSize - 1, 0); // Leave space for null terminator
    //    if (bytesReceived > 0) {
    //        buffer[bytesReceived] = '\0'; // Null-terminate the received data
    //        std::string receivedData(buffer);

    //        // Split the received data into separate strings
    //        std::istringstream iss(receivedData);
    //        std::string line;
    //        while (std::getline(iss, line, '\n')) { // Use '\n' as the delimiter
    //            timestamp ts;
    //            std::cout << line << std::endl;
    //        }
    //    }
    //} while (bytesReceived > 0);

    closesocket(server_socket);
    WSACleanup();

    return 0;
}
