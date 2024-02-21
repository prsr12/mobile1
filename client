#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

constexpr int MAX_BUFFER_SIZE = 1024;
constexpr int MAX_FILE_SIZE = 100 * 1024 * 1024; // 100 MiB

class Client {
public:
    Client(const std::string& hostname, unsigned short port, const std::string& filename)
        : hostname(hostname), port(port), filename(filename), timedOut(false) {
        signal(SIGALRM, handleSignal);
        connectToServer();
    }

private:
    std::string hostname;
    unsigned short port;
    std::string filename;
    bool timedOut;

    static void handleSignal(int signum) {
        if (signum == SIGALRM) {
            std::cerr << "ERROR: Connection timed out." << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    void connectToServer() {
        int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == -1) {
            std::cerr << "ERROR: Unable to create client socket." << std::endl;
            std::exit(EXIT_FAILURE);
        }

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        if (inet_pton(AF_INET, hostname.c_str(), &serverAddr.sin_addr) <= 0) {
            std::cerr << "ERROR: Invalid hostname or IP address." << std::endl;
            close(clientSocket);
            std::exit(EXIT_FAILURE);
        }

        alarm(10); // Set a timeout for connection
        if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
            std::cerr << "ERROR: Unable to connect to the server." << std::endl;
            close(clientSocket);
            std::exit(EXIT_FAILURE);
        }
        alarm(0); // Disable the timeout

        sendFile(clientSocket);

        close(clientSocket);
    }

    void sendFile(int clientSocket) {
        std::ifstream fileStream(filename, std::ios::binary);
        if (!fileStream.is_open()) {
            std::cerr << "ERROR: Unable to open the specified file." << std::endl;
            std::exit(EXIT_FAILURE);
        }

        char buffer[MAX_BUFFER_SIZE];
        while (!fileStream.eof()) {
            fileStream.read(buffer, sizeof(buffer));
            ssize_t bytesSent = send(clientSocket, buffer, fileStream.gcount(), 0);
            if (bytesSent == -1) {
                std::cerr << "ERROR: Unable to send data to the server." << std::endl;
                close(clientSocket);
                std::exit(EXIT_FAILURE);
            }
        }

        fileStream.close();

       
        char response[MAX_BUFFER_SIZE];
        ssize_t bytesReceived = recv(clientSocket, response, sizeof(response), 0);
        if (bytesReceived == -1) {
            std::cerr << "ERROR: Failed to receive response from the server." << std::endl;
            close(clientSocket);
            std::exit(EXIT_FAILURE);
        } else if (bytesReceived == 0) {
            std::cerr << "ERROR: Connection closed by the server." << std::endl;
            close(clientSocket);
            std::exit(EXIT_FAILURE);
        }

        
        std::cout << "Server response: " << std::string(response, bytesReceived) << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "ERROR: Invalid number of arguments." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::string hostname = argv[1];
    unsigned short port = std::atoi(argv[2]);
    std::string filename = argv[3];

    if (port == 0) {
        std::cerr << "ERROR: Invalid port number." << std::endl;
        std::exit(EXIT_FAILURE);
    }

    Client client(hostname, port, filename);

    std::cout << "File transfer completed successfully." << std::endl;

    return 0;
}

