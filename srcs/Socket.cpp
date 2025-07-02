#include "include/Socket.hpp"
#include "Logger.hpp"
#include "Client.hpp"
#include "Socket.hpp"
#include "ServerManager.hpp"

void Socket::shutdown() {
	close(this->_server_fd);
	Logger::info("Shutdown server.");
}
void ServerManager::shutdown_epoll() {
	close(this->epoll_fd);
	Logger::info("epoll function closed. Shutdown complete.");
}
Socket::Socket(int port) 
: _server_fd(-1), _port(port), _addrlen(sizeof(_address)), isrunning(true)
{
	std::memset(&_address, 0, sizeof(_address));
}

Socket::~Socket()
{
	if (_server_fd != -1)
	{
		close(_server_fd);
	}
}

bool Socket::setup()
{
	// man socket
	// domain: AF_INET == IPV4
	// type: AF_INET == two way connection
	// protocol: we can leave as default
	Logger::info("Creating socket");
	_server_fd = socket(AF_INET, SOCK_STREAM, 0); // returns a file descriptor that refers to that endpoint(just like open())
	if (_server_fd < 0)
	{
		throw std::runtime_error("Socket creation failed");
	}

	int opt = 1;
	Logger::info("Setting sersockopt");
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		throw std::runtime_error("Setsockopt failed");
	}
	
	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = INADDR_ANY;
	_address.sin_port = htons(_port);

	Logger::info("Binding socket");
	if (bind(_server_fd, (sockaddr *)&_address, sizeof(_address)) == -1)
	{
		throw std::runtime_error("Bind failed");
	}

	Logger::info("Listening on socket");
	if (listen(_server_fd, 10) == -1)
	{
		throw std::runtime_error("Listen failed");
	}
	Logger::debug("Server listening on port " + intToString(_port));
	isrunning = true;
	return true;
}

int Socket::acceptClient()
{
	int fd = accept(_server_fd, (sockaddr *)&_address, &_addrlen); 
	return fd;
}
// Waits for client to connect. 
// Returns a new file descriptor - used to communicate with that spcific client.
// This is typically called in a loop to keep accepting new connections.

int Socket::response(const Client& client)
{
    int bytesSent = send(client.getClientFd(), this->res.c_str(), this->res.length(), 0);

    if (bytesSent < 0)
        Logger::error("Failed to send response to client FD " + intToString(client.getClientFd()) + ": " + std::string(strerror(errno)));
    else 
	{
        Logger::info("Sent " + intToString(bytesSent) + " bytes to client FD " + intToString(client.getClientFd()));
    }
    return bytesSent;
}
// Sends the current res string to a specific client.
// Uses the client's socket file descriptor.
// send(...) is a wrapper over low-level write call, used for sockets.

int Socket::getServerFd() const
{
	return _server_fd;
}
// Getter for main listening socket FD.
// Useful for using this fd in a select()/poll()/epoll() loop or to monitor it externally.

void Socket::setResponse(std::string res)
{
	this->res = res;
}
// Assigns a response string to the res field.
// Prepares the Socket object to send this response later using response().

void Socket::closeServerSocket() {
	if (this->_server_fd != -1) {
		close(this->_server_fd);
		this->_server_fd = -1;
		std::cout << "Server socket closed safely.\n";
	}
}


void set_non_blocking(int fd) 
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) 
    {
        Logger::error("Failed to get flags for fd " + intToString(fd));
        throw std::runtime_error("fcntl F_GETFL failed");
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) 
    {
        Logger::error("Failed to set O_NONBLOCK for fd " + intToString(fd));
        throw std::runtime_error("fcntl F_SETFL failed");
    }
    Logger::info("Set fd " + intToString(fd) + " to non-blocking mode");
}
// F_GETFL - is a command used with the fcntl() function 
// in Linux to retrieve the file status flags associated with a file descriptor.