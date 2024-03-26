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

std::mutex oxygenMtx;
std::mutex hydrogenMtx;
std::mutex coutMutex; // Mutex for protecting std::cout operations
std::vector<Log> oxygenVector;
std::vector<Log> hydrogenVector;
std::vector<std::string> waterVector;

bool isServerRunning = true;

void acceptClient(SOCKET client_socket, int atom) {
	const int bufferSize = 2048;
	char buffer[bufferSize];
	int bytesReceived;
    timestamp ts;
    std::string tempString;

    do {
		bytesReceived = recv(client_socket, buffer, bufferSize - 1, 0); // Leave space for null terminator
        if (bytesReceived > 0) {
			buffer[bytesReceived] = '\0'; // Null-terminate the received data
			std::string receivedData(buffer);

			// Split the received data into separate strings
			std::istringstream iss(receivedData);
			std::string line;
			while (std::getline(iss, line, '\n')) { // Use '\n' as the delimiter
                
                {
                    std::lock_guard<std::mutex> coutLock(coutMutex); // Protect std::cout operation
                    std::cout << line << ", " << ts.getCurrentTime() << std::endl;
                }
                Log log;
                if (atom == 0) {
                    if ((line.find('{') != std::string::npos) && (line.find('}') != std::string::npos)) {
                        std::lock_guard<std::mutex> lock(oxygenMtx);
                        line = line.substr(1, line.size() - 2);
                        std::istringstream iss(line);
                        std::getline(iss, log.id, ',');
                        std::getline(iss, log.type);
                        oxygenVector.push_back(log);
                    }
                    else {
                        if (tempString == "") {
                            tempString = line;
                        }
                        else {
                            tempString = tempString + line;
							std::lock_guard<std::mutex> lock(oxygenMtx);
                            tempString = tempString.substr(1, tempString.size() - 2);
							std::istringstream iss(tempString);
							std::getline(iss, log.id, ',');
							std::getline(iss, log.type);
							oxygenVector.push_back(log);
							tempString = "";
                        }   
					}
				}
				else if (atom == 1) {
                    if ((line.find('{') != std::string::npos) && (line.find('}') != std::string::npos)) {
                        std::lock_guard<std::mutex> lock(hydrogenMtx);
                        line = line.substr(1, line.size() - 2);
                        std::istringstream iss(line);
                        std::getline(iss, log.id, ',');
                        std::getline(iss, log.type);
                        hydrogenVector.push_back(log);
                    }
                    else {
                        if (tempString == "") {
                            tempString = line;
                        }
                        else {
                            tempString = tempString + line;
                            std::lock_guard<std::mutex> lock(hydrogenMtx);
                            tempString = tempString.substr(1, tempString.size() - 2);
                            std::istringstream iss(tempString);
                            std::getline(iss, log.id, ',');
                            std::getline(iss, log.type);
                            hydrogenVector.push_back(log);
                            tempString = "";
                        }
                    }
				}
            }
		}
        else if (bytesReceived == 0) {
            //std::lock_guard<std::mutex> coutLock(coutMutex); // Protect std::cout operation
            std::cout << "Connection closed by peer.\n";
            isServerRunning = false;
        }
        else {
            //std::lock_guard<std::mutex> coutLock(coutMutex); // Protect std::cerr operation
            std::cerr << "Receive failed.\n";
        }
	} while (bytesReceived > 0);

	closesocket(client_socket);
}

void bindAtoms(SOCKET oSocket, SOCKET hSocket) {
    while (isServerRunning) {
        oxygenMtx.lock();
        if (oxygenVector.size() >= 1) {
            hydrogenMtx.lock();
            if (hydrogenVector.size() >= 2) {
                timestamp ts;
                std::string oxygenLogString = oxygenVector[0].id + ", bonded, " + ts.getCurrentTime() + "\n";
                std::string hydrogenLogString1 = hydrogenVector[0].id + ", bonded, " + ts.getCurrentTime() + "\n";
                std::string hydrogenLogString2 = hydrogenVector[1].id + ", bonded, " + ts.getCurrentTime() + "\n";
                {
                    std::lock_guard<std::mutex> coutLock(coutMutex);
                    std::cout << oxygenVector[0].id + " | " + hydrogenVector[0].id + " | " + hydrogenVector[1].id + "\n";
                }
                /*{
                    std::lock_guard<std::mutex> coutLock(coutMutex);
                    std::cout << oxygenLogString;
                }
                {
                    std::lock_guard<std::mutex> coutLock(coutMutex);
                    std::cout << hydrogenLogString1;
                }
                {
                    std::lock_guard<std::mutex> coutLock(coutMutex);
                    std::cout << hydrogenLogString2;
                }*/

                send(oSocket, oxygenLogString.c_str(), oxygenLogString.size(), 0);
                send(hSocket, hydrogenLogString1.c_str(), hydrogenLogString1.size(), 0);
                send(hSocket, hydrogenLogString2.c_str(), hydrogenLogString2.size(), 0);

                oxygenVector.erase(oxygenVector.begin());
                hydrogenVector.erase(hydrogenVector.begin(), hydrogenVector.begin() + 2); // +2 because it is exclusive.
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

    // Create a socket for O client
    SOCKET oClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (oClientSocket == INVALID_SOCKET) {
        std::cout << "Oxygen client socket creation failed." << std::endl;
        WSACleanup();
        return 1;
    }

    // Create a socket for H client
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
    {
        std::lock_guard<std::mutex> coutLock(coutMutex);
        std::cout << "Server is running..." << std::endl;
    }

    SOCKET o_client_socket;
    int oClientAddrSize = sizeof(oClientAddr);
    o_client_socket = accept(oClientSocket, reinterpret_cast<sockaddr*>(&oClientAddr), &oClientAddrSize);
    if (o_client_socket == INVALID_SOCKET) {
        std::cerr << "Accept failed.\n";
        closesocket(oClientSocket);
        WSACleanup();
        return 3;
    }

    SOCKET h_client_socket;
    int hClientAddrSize = sizeof(hClientAddr);
    h_client_socket = accept(hClientSocket, reinterpret_cast<sockaddr*>(&hClientAddr), &hClientAddrSize);
    if (h_client_socket == INVALID_SOCKET) {
        std::cerr << "Accept failed.\n";
        closesocket(hClientSocket);
        WSACleanup();
        return 3;
    }

    
    std::thread acceptOxygenClientThread(acceptClient, o_client_socket, 0);
    std::thread acceptHydrogenClientThread(acceptClient, h_client_socket, 1);
    std::thread bindAtomsThread(bindAtoms, o_client_socket, h_client_socket);

    acceptOxygenClientThread.detach();
    acceptHydrogenClientThread.detach();
    bindAtomsThread.detach();

    while (isServerRunning) {

    }

    closesocket(oClientSocket);
    closesocket(hClientSocket);
    WSACleanup();
    return 0;
}
