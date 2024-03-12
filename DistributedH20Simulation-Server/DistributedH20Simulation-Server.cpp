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
//#include <log.hpp>

#pragma comment(lib, "ws2_32.lib")

struct Log {
    std::string id;
    std::string type;
};

std::mutex oxygenMtx;
std::mutex hydrogenMtx;
std::vector<Log> oxygenVector;
std::vector<Log> hydrogenVector;

void acceptClient(SOCKET clientSocket, int atom) {
	SOCKET client_socket;
	sockaddr_in clientAddr;
	int clientAddrSize = sizeof(clientAddr);
	client_socket = accept(clientSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrSize);
    if (client_socket == INVALID_SOCKET) {
		std::cerr << "Accept failed.\n";
		closesocket(clientSocket);
		WSACleanup();
		return;
	}

	const int bufferSize = 1024;
	char buffer[bufferSize];
	int bytesReceived;

    do {
		bytesReceived = recv(client_socket, buffer, bufferSize - 1, 0); // Leave space for null terminator
        if (bytesReceived > 0) {
			buffer[bytesReceived] = '\0'; // Null-terminate the received data
			std::string receivedData(buffer);

			// Split the received data into separate strings
			std::istringstream iss(receivedData);
			std::string line;
			while (std::getline(iss, line, '\n')) { // Use '\n' as the delimiter
                std::cout << "Received: " << line << std::endl;
                Log log;
                if (atom == 0) {
                    std::lock_guard<std::mutex> lock(oxygenMtx);
					std::istringstream iss(line);
                    std::getline(iss, log.id, ','); 
                    std::getline(iss, log.type);
					oxygenVector.push_back(log);
				}
				else if (atom == 1) {
					std::lock_guard<std::mutex> lock(hydrogenMtx);
					std::istringstream iss(line);
                    std::getline(iss, log.id, ',');
                    std::getline(iss, log.type);
					hydrogenVector.push_back(log);
				}
            }
		}
        else if (bytesReceived == 0) {
			std::cout << "Connection closed by peer.\n";
		}
        else {
			std::cerr << "Receive failed.\n";
		}
	} while (bytesReceived > 0);

	closesocket(client_socket);
}

void bindAtoms() {
    while (true) {
        oxygenMtx.lock();
        if (oxygenVector.size() >= 1) {
            hydrogenMtx.lock();
            if (hydrogenVector.size() >= 2) {
                std::cout << "Binding atoms..." << std::endl;
                std::cout << "Oxygen: " << oxygenVector[0].id << std::endl;
                std::cout << "Hydrogen: " << hydrogenVector[0].id << ", " << hydrogenVector[1].id << std::endl;
                oxygenVector.erase(oxygenVector.begin());
                hydrogenVector.erase(hydrogenVector.begin(), hydrogenVector.begin() + 2);
            }
            hydrogenMtx.unlock();
        }
        oxygenMtx.unlock();
    }
}

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

    
    std::thread acceptOxygenClientThread(acceptClient, oClientSocket, 0);
    acceptOxygenClientThread.join();

    std::thread acceptHydrogenClientThread(acceptClient, hClientSocket, 1);
    acceptHydrogenClientThread.join();

    std::thread bindAtomsThread(bindAtoms);
    bindAtomsThread.join();


    closesocket(oClientSocket);
    closesocket(hClientSocket);
    WSACleanup();
    return 0;
}
