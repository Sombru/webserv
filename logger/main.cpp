#include "Logger.hpp"

int main() {
    Logger::info("Initializing Server...");

    std::string serverName = "localhost";
    std::string host = "127.0.0.1";
    int port = 8002;

    std::cout << " Calling simple message function: \n";
    Logger::info("Server Created: ");
    std::cout << "ServerName[" << serverName << "] "
              << "Host[" << host << "] "
              << "Port[" << port << "]"
              <<std::endl;

    std::cout << "Calling overloaded function: \n";
    Logger::info("Server Created: ", serverName, host, port);
    
    return (0);
}