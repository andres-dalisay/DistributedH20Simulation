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

#pragma comment(lib, "ws2_32.lib")

#define MASTER_SERVER_IP "127.0.0.1"
#define H_LIMIT 2097152

//void send_task(const char* start_point, const char* end_point) {
//
//    clock_t start, end;
//
//
//    char task[256];
//    snprintf(task, sizeof(task), "%s,%s", start_point, end_point);
//    send(client_socket, task, strlen(task), 0);
//    start = clock();
//
//    std::vector<char> buffer(100000000);
//    int bufferBytes = recv(client_socket, buffer.data(), buffer.size(), 0);
//    end = clock();
//
//
//    buffer.resize(bufferBytes);
//    std::vector<int> primes = deserializeVector(buffer);
//
//    std::cout << "Number of primes: " << primes.size() << std::endl;
//
//    double time_taken = double(end - start) / double(CLOCKS_PER_SEC);
//    std::cout << "Time taken by program is : " << std::fixed << time_taken << std::setprecision(5) << std::endl;
//
//    if (SORT_ARRAY) {
//        std::sort(primes.begin(), primes.end());
//    }
//
//    if (DISPLAY_ARRAY)
//    {
//        std::cout << "Primes: ";
//        for (int prime : primes) {
//            std::cout << prime << " ";
//        }
//        std::cout << std::endl;
//    }
//
//    closesocket(client_socket);
//    WSACleanup();
//}

struct Log {
    std::string id;
    std::string type;
};

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

    int h_int;

    while (true) {
        std::cout << "Enter number of hydrogen atoms: ";
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
    }

    std::vector<Log> logs;

    freopen("hydrogen_log.txt", "w", stdout);
    for (int i = 1; i <= h_int; i++) {
        Log log;
        log.id = "H" + std::to_string(i);
        log.type = "request";
        logs.push_back(log);
        // get the current timestamp
        auto now = std::chrono::system_clock::now();
        // print the current timestamp
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        std::tm* local_time = std::localtime(&now_c);
        std::ostringstream oss;
        oss << std::put_time(local_time, "%Y-%m-%d %H:%M:%S");
        std::string formatted_time = oss.str();
        
        std::printf("%s, %s, %s\n", log.id.c_str(), log.type.c_str(), formatted_time.c_str());
        
        //send(server_socket, reinterpret_cast<char*>(&log), sizeof(log), 0);
    }
    fclose(stdout);

    closesocket(server_socket);

    return 0;
}