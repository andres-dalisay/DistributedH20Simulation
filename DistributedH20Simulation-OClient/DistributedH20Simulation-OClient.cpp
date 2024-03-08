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
#include <cereal/archives/binary.hpp>
#include <log.hpp>

#pragma comment(lib, "ws2_32.lib")

#define MASTER_SERVER_IP "127.0.0.1"
#define H_LIMIT 1048576




// Function to serialize a vector of integers into a byte stream
std::vector<char> serializeVector(const std::vector<int>& vec) {
    std::vector<char> bytes;
    // Assuming integers are 4 bytes each
    for (int num : vec) {
        // Convert each integer to bytes
        char* numBytes = reinterpret_cast<char*>(&num);
        for (size_t i = 0; i < sizeof(num); ++i) {
            bytes.push_back(numBytes[i]);
        }
    }
    return bytes;
}

// Function to deserialize a byte stream into a vector of integers
std::vector<int> deserializeVector(const std::vector<char>& bytes) {
    std::vector<int> vec;
    // Assuming integers are 4 bytes each
    for (size_t i = 0; i < bytes.size(); i += sizeof(int)) {
        int num;
        // Convert bytes back to integer
        memcpy(&num, &bytes[i], sizeof(int));
        vec.push_back(num);
    }
    return vec;
}

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

    while (true) {
        std::cout << "Enter number of oxygen atoms: ";
        std::cin.getline(oInput, sizeof(oInput));


        try {

            o_int = std::stoi(oInput);
            if (o_int < 0 || o_int > H_LIMIT) {
                std::cerr << "Error: Invalid input. ";
                if (o_int < 0) {
                    std::cerr << "Input must be positive. ";
                }
                if (o_int > H_LIMIT) {
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
        Log log;
        log.id = "O" + std::to_string(i);
        log.type = "request";
        std::stringstream ss;
        {
            cereal::BinaryOutputArchive oarchive(ss);
            oarchive(log);
        }
        // get the current timestamp
        auto now = std::chrono::system_clock::now();
        // print the current timestamp
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm* local_time = std::localtime(&now_c);
        std::ostringstream oss;
        oss << std::put_time(local_time, "%Y-%m-%d %H:%M:%S");
        std::string formatted_time = oss.str();

        std::printf("%s, %s, %s\n", log.id.c_str(), log.type.c_str(), formatted_time.c_str());

        // Serialize the log struct
        std::string serializedLog = ss.str();
        // Send the serialized log data to the server
        send(server_socket, serializedLog.c_str(), serializedLog.size(), 0);

	}
    fclose(stdout);

    closesocket(server_socket);
    WSACleanup();

    return 0;
}
