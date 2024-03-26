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
#include <fstream>
#include <sstream>
#include <log.hpp>
#include <thread>
#include <set>

#pragma comment(lib, "ws2_32.lib")

#define MASTER_SERVER_IP "127.0.0.1"
#define H_LIMIT 2097152

int h_int = 200;
clock_t start, end;

void sendData(SOCKET server_socket) {
    timestamp ts;

    std::ofstream logFile("hydrogen_log_sent.txt");
    for (int i = 1; i <= h_int; i++) {
        std::string currentTime = ts.getCurrentTime(); // Gets Current Timestamp to print on Logs
        std::string logString = "H" + std::string(std::to_string(i)) + ", request";
        logFile << logString << ", " << currentTime << std::endl;

        // Just for formatting on the server
        logString = "{" + logString + "}\n";

        // Send the serialized log data to the server
        int bytesSent = send(server_socket, logString.c_str(), logString.size(), 0);
        if (i == 1) {
            start = clock();
        }
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Failed to send data.\n";
            closesocket(server_socket);
            WSACleanup();
            break;
        }
    }
    logFile.close();
}

void receiveData(SOCKET server_socket) {
    const int bufferSize = 2048;
    char buffer[bufferSize];
    int bytesReceived;
    int ctr = 0;
    std::ofstream logFile("hydrogen_log_received.txt");
    do {
        bytesReceived = recv(server_socket, buffer, bufferSize - 1, 0); // Attempt to receive data, leaving space for null terminator
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0'; // Null-terminate the received data to safely treat it as a string

            std::string receivedData(buffer);
            std::istringstream iss(receivedData);
            std::string line;

            while (std::getline(iss, line, '\n')) { // Process each line separated by '\n'
                timestamp ts; // Assuming you want to use this for something, e.g., logging with a timestamp
                std::string currentTime = ts.getCurrentTime(); // Get the current time as a string

                // Log the received line with the current timestamp to the console or a file
                std::cout << "Received at " << currentTime << ": " << line << std::endl;
                logFile << line << std::endl; // This is what writes the log on the file
                ctr++;
            }
        }
        else if (bytesReceived == 0) {
            std::cout << "Connection closed by the client." << std::endl;
            break; // Exit the loop if the connection has been closed
        }
        else {
            std::cerr << "Receive failed with error code: " << WSAGetLastError() << std::endl;
            break; // Exit the loop if an error occurred
        }
    } while (bytesReceived > 0 && ctr < h_int);
    logFile.close();
    end = clock();
}

int main() {
    char hInput[100];

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
    server_address.sin_port = htons(5000);
    inet_pton(AF_INET, MASTER_SERVER_IP, &server_address.sin_addr);

    if (connect(server_socket, reinterpret_cast<SOCKADDR*>(&server_address), sizeof(server_address)) == SOCKET_ERROR) {
        std::cerr << "Error connecting to server" << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 3;
    }

  
    /*while (true) {
        std::cout << "Enter number of hydrogen atoms (LIMIT = " << H_LIMIT << "): ";
        std::cin.getline(hInput, sizeof(hInput));

        try {
            h_int = std::stoi(hInput);
            if (h_int < 0 || h_int > H_LIMIT) {
                std::cerr << "Error: Invalid input. ";
                if (h_int < 0) {
                    std::cerr << "Input must be positive. ";
                }
                if (h_int > H_LIMIT) {
                    std::cerr << "Input must be less than 2097152. ";
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
    }*/

    std::cout << "Logging Hydrogen..." << std::endl;
    
    std::thread sendThread(sendData, server_socket);
    std::thread receiveThread(receiveData, server_socket);

    sendThread.join();
    receiveThread.join();

    double time_taken = double(end - start) / double(CLOCKS_PER_SEC);
    std::cout << "Time taken by program is : " << std::fixed << time_taken << std::setprecision(5) << std::endl;


    std::ifstream hydrogenSentFile("hydrogen_log_sent.txt"); // Open the file
    std::ifstream hydrogenReceivedFille("hydrogen_log_received.txt");

    if (!hydrogenSentFile && !hydrogenReceivedFille) {
        std::cerr << "Error opening file.\n";
        return 1;
    }

    std::set<std::string> uniqueLinesSent; // Set to store unique lines
    std::set<std::string> uniqueLinesReceived;
    std::string line;

    // Read lines from the file
    while (std::getline(hydrogenSentFile, line)) {
        // Check if the line is already present in the set
        if (uniqueLinesSent.find(line) != uniqueLinesSent.end()) {
            std::cout << "Duplicate line: " << line << std::endl;
            break;
        }
        else {
            uniqueLinesSent.insert(line); // Insert the line into the set
        }
    }

    while (std::getline(hydrogenReceivedFille, line)) {
        if (uniqueLinesReceived.find(line) != uniqueLinesReceived.end()) {
            std::cout << "Duplicate line: " << line << std::endl;
            break;
        }
        else {
            uniqueLinesReceived.insert(line);
        }
    }

    closesocket(server_socket);
    WSACleanup();

    return 0;
}