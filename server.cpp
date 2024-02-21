#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>

constexpr int MAX_BUFFER_SIZE = 1024;
constexpr int MAX_FILE_SIZE = 100 * 1024 * 1024; // 100 MiB

class Server {
public:
    Server(unsigned short port, const std::string& fileDir)
        : port(port), fileDir(fileDir), connectionId(0), timedOut(false) {
        signal(SIGQUIT, handleSignal);
        signal(SIGTERM, handleSignal);
        startServer();
    }

private:
    unsigned short port;
    std::string fileDir;
    unsigned int connectionId;
    bool timedOut;

    static void handleSignal(int signum) {
        if (signum == SIGQUIT || signum == SIGTERM) {
            std::exit(EXIT_SUCCESS);
        }
    }

    std::string getServerIP() const {
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock == -1) {
            std::cerr << "ERROR: Unable to create a temporary socket" << std::endl;
            std::exit(EXIT_FAILURE);
        }

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = inet_addr("8.8.8.8");
        serverAddr.sin_port = htons(53);

        if (connect(sock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
            std::cerr << "ERROR: Unable to connect to the temporary socket" << std::endl;
            close(sock);
            std::exit(EXIT_FAILURE);
        }

        sockaddr_in localAddr{};
        socklen_t addrLen = sizeof(localAddr);
        getsockname(sock, reinterpret_cast<sockaddr*>(&localAddr), &addrLen);

        close(sock);

        return inet_ntoa(localAddr.sin_addr);
    }

    void sendAck(int clientSocket, const std::string& message) const {
        send(clientSocket, message.c_str(), message.size(), 0);
    }

    void startServer() {
        std::string serverIP = getServerIP();
        std::cout << "Server IP: " << serverIP << std::endl;

        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1) {
            std::cerr << "ERROR: Unable to create server socket." << std::endl;
            std::exit(EXIT_FAILURE);
        }

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);

        if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
            std::cerr << "ERROR: Unable to bind to port." << std::endl;
            close(serverSocket);
            std::exit(EXIT_FAILURE);
        }

        listen(serverSocket, 1);

        while (true) {
            int clientSocket;
            timeval timeout{};
            timeout.tv_sec = 60;

            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(serverSocket, &readfds);

            int activity = select(serverSocket + 1, &readfds, nullptr, nullptr, &timeout);

            if (activity == 0) {
                std::cout << "Inactivity of 60 seconds. Shutting down the server." << std::endl;
                break;
            }

            if (FD_ISSET(serverSocket, &readfds)) {
                clientSocket = accept(serverSocket, nullptr, nullptr);
                if (clientSocket == -1) {
                    std::cerr << "ERROR: Unable to accept connection." << std::endl;
                    continue; 
                }

                connectionId++;
                std::cout << "Connection " << connectionId << " accepted." << std::endl;
                sendAck(clientSocket, "ACK Connection established");

                std::string fileName = fileDir + "/" + std::to_string(connectionId) + ".file";

                std::ofstream fileStream(fileName, std::ios::binary);

                timedOut = false;
                handleConnection(clientSocket, fileStream);

                fileStream.close();
                sendAck(clientSocket, "ACK File received successfully");

                close(clientSocket);
            }
        }

        close(serverSocket);
    }

    void handleConnection(int clientSocket, std::ofstream& fileStream) {
        ssize_t bytesRead;
        char buffer[MAX_BUFFER_SIZE];

        while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
            fileStream.write(buffer, bytesRead);
        }

        if (bytesRead == -1) {
            std::cerr << "ERROR: Unable to receive data from the client." << std::endl;
            std::exit(EXIT_FAILURE);
        }

        // Check if the received file exceeds the maximum allowed size
        if (fileStream.tellp() > MAX_FILE_SIZE) {
            std::cerr << "ERROR: File size exceeds the maximum allowed size." << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "ERROR: Invalid number of arguments." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    unsigned short port = std::atoi(argv[1]);
    std::string fileDir = argv[2];

    if (port == 0) {
        std::cerr << "ERROR: Invalid port number." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    Server server(port, fileDir);

    return 0;
}

