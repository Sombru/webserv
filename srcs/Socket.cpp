#include "Socket.hpp"
#include "Logger.hpp"
#include "Client.hpp"

// Socket::Socket(const ServerConfig &server)
// 	: server_fd(-1),
// 	  port(server.port),
// 	  addres_str(server.host),
// 	  response(),
// 	  address(),
// 	  addrlen(sizeof(address)),
// 	  request_size(server.client_max_body_size),
// 	  running(true)
// {
// 	std::memset(&address, 0, sizeof(address));
// }

// Socket::~Socket()
// {
// 	if (server_fd > 0)
// 	{
// 		close(server_fd);
// 	}
// }

// bool Socket::setup()
// {
// 	// man socket
// 	// domain: AF_INET == IPV4
// 	// type: SOCK_STREAM == two way connection
// 	// protocol: we can leave as default
// 	Logger::info("Creating socket");
// 	server_fd = socket(AF_INET, SOCK_STREAM, 0); // returns a file descriptor that refers to that endpoint(just like open())
// 	if (server_fd < 0)
// 	{
// 		throw std::runtime_error("Socket creation failed");
// 	}

// 	int opt = 1;
// 	Logger::info("Setting sersockopt");
// 	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
// 	{
// 		throw std::runtime_error("Setsockopt failed");
// 	}
// 	address.ai_family = AF_INET;
// 	address.ai_socktype = SOCK_STREAM;
// 	address.ai_flags = AI_PASSIVE;

// 	struct addrinfo *res;
// 	getaddrinfo(this->addres_str.c_str(), intToString(this->port).c_str(), &address, &res);
// 	Logger::info("Binding socket");
// 	if (bind(server_fd, res->ai_addr, res->ai_addrlen) == -1)
// 	{
// 		throw std::runtime_error("Bind failed");
// 	}

// 	Logger::info("Listening on socket");
// 	if (listen(server_fd, 10) == -1)
// 	{
// 		throw std::runtime_error("Listen failed");
// 	}
// 	Logger::debug("Server listening on port " + intToString(port));
// 	return true;
// }

// int Socket::respond(const Client &client)
// {
// 	int bytesSent = send(client.getClientFd(), this->response.c_str(), this->response.length(), 0);

// 	if (bytesSent < 0)
// 		Logger::error("Failed to send response to client FD " + intToString(client.getClientFd()) + ": " + std::string(strerror(errno)));
// 	else
// 	{
// 		Logger::info("Sent " + intToString(bytesSent) + " bytes to client FD " + intToString(client.getClientFd()));
// 	}
// 	return bytesSent;
// }

// int Socket::acceptClient()
// {
// 	int fd = accept(server_fd, (sockaddr *)&address, &addrlen);
// 	return fd;
// }

// int Socket::getServerFD() const
// {
// 	return this->server_fd;
// }

// int Socket::getRequestSize() const
// {
// 	return this->request_size;
// }

// void Socket::closeServerSocket()
// {
// 	if (this->server_fd != -1)
// 	{
// 		close(this->server_fd);
// 		this->server_fd = -1;
// 		std::cout << "Server socket closed safely.\n";
// 	}
// }

bool Socket::setup()
{
    Logger::info("Creating socket");
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
        throw std::runtime_error("Socket creation failed");

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        throw std::runtime_error("setsockopt failed");

    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* res;
    std::string portStr = intToString(this->port);

    int status = getaddrinfo(this->host.c_str(), portStr.c_str(), &hints, &res);
    if (status != 0 || !res)
        throw  std::runtime_error("getaddrinfo failed");

    if (bind(server_fd, res->ai_addr, res->ai_addrlen) == -1) {
        freeaddrinfo(res);
        throw std::runtime_error("Bind failed");
    }

    freeaddrinfo(res);

    if (listen(server_fd, 10) == -1)
        throw std::runtime_error("Listen failed");
    
    Logger::info("Listening on port " + portStr);
    return (true);
}

int Socket::getServerFd() const {
    return (this->server_fd);
}
