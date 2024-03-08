// master_server.cpp
#include <iostream>
#include <winsock2.h>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <iomanip>
#include <cereal/archives/binary.hpp>
#include <log.hpp>

#pragma comment(lib, "ws2_32.lib")


std::mutex mtx;
std::vector<int> primes;


int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Error initializing Winsock" << std::endl;
        return -1;
    }

    // Create a socket for clients
    SOCKET oClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (oClientSocket == INVALID_SOCKET) {
        std::cout << "Oxygen client socket creation failed." << std::endl;
        WSACleanup();
        return 1;
    }

    // Create a socket for clients
    SOCKET hClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (hClientSocket == INVALID_SOCKET) {
        std::cout << "Hydrogen client socket creation failed." << std::endl;
        WSACleanup();
        return 1;
    }

    // Bind the client socket to an address and port
    sockaddr_in oClientAddr;
    oClientAddr.sin_family = AF_INET;
    oClientAddr.sin_port = htons(5001); // Port number
    oClientAddr.sin_addr.s_addr = INADDR_ANY; // Accept connections from any address

    if (bind(oClientSocket, reinterpret_cast<sockaddr*>(&oClientAddr), sizeof(oClientAddr)) == SOCKET_ERROR) {
        std::cout << "Oxygen client socket bind failed." << std::endl;
        closesocket(oClientSocket);
        WSACleanup();
        return 1;
    }


    // Bind the client socket to an address and port
    sockaddr_in hClientAddr;
    hClientAddr.sin_family = AF_INET;
    hClientAddr.sin_port = htons(5000); // Port number
    hClientAddr.sin_addr.s_addr = INADDR_ANY; // Accept connections from any address

    if (bind(hClientSocket, reinterpret_cast<sockaddr*>(&hClientAddr), sizeof(hClientAddr)) == SOCKET_ERROR) {
        std::cout << "Hydrogen client socket bind failed." << std::endl;
        closesocket(hClientSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections on the client socket
    if (listen(oClientSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cout << "Oxygen socket listen failed." << std::endl;
        closesocket(oClientSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections on the slave socket
    if (listen(hClientSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cout << "Hydrogen socket listen failed." << std::endl;
        closesocket(hClientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is running..." << std::endl;







    while (true) {
        SOCKET o_client_socket = accept(oClientSocket, NULL, NULL);
        if (o_client_socket == INVALID_SOCKET) {
            std::cerr << "Error accepting connection" << std::endl;
        }
        SOCKET h_client_socket = accept(hClientSocket, NULL, NULL);
        if (h_client_socket == INVALID_SOCKET) {
            std::cerr << "Error accepting connection" << std::endl;
        }
        std::string o_molecule;
        // receive a string from the o client
        const int bufferSize = 1024;
        char buffer[bufferSize];
        int bytesReceived = recv(o_client_socket, buffer, bufferSize, 0);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "Error receiving data" << std::endl;
        }
        std::istringstream iss(std::string(buffer, bytesReceived));
        Log receivedLog;
        {
            cereal::BinaryInputArchive archive(iss);
            archive(receivedLog);
        }
        std::cout << "Received: " << receivedLog.id << " " << receivedLog.type << std::endl;
        // clear buffer
        memset(buffer, 0, bufferSize);
    }

    closesocket(oClientSocket);
    closesocket(hClientSocket);
    WSACleanup();
    return 0;
}
