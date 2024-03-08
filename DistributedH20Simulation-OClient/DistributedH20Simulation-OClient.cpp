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
#define H_LIMIT 1048576

//using time_point = std::chrono::system_clock::time_point;
//std::string serializeTimePoint(const time_point& time, const std::string& format)
//{
//    std::time_t tt = std::chrono::system_clock::to_time_t(time);
//    std::tm tm = *std::gmtime(&tt); //GMT (UTC)
//    //std::tm tm = *std::localtime(&tt); //Locale time-zone, usually UTC by default.
//    std::stringstream ss;
//    ss << std::put_time(&tm, format.c_str());
//    return ss.str();
//}

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
    std::vector<Log> logs;

    freopen("oxygen_log.txt", "w", stdout);
    for (int i = 1; i <= o_int; i++) {
        Log log;
		log.id = "O" + std::to_string(i);
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
