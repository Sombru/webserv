#include "include/Socket.hpp"
#include "Logger.hpp"
#include "Client.hpp"

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
	Logger::debug("Creating socket");
	_server_fd = socket(AF_INET, SOCK_STREAM, 0); // returns a file descriptor that refers to that endpoint(just like open())
	if (_server_fd < 0)
	{
		Logger::error("Socket creation failed");
		throw std::runtime_error("failed to setup the server");
	}

	int opt = 1;
	Logger::debug("Setting sersockopt");
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		Logger::error("setsockopt failed");
		throw std::runtime_error("failed to setup the server");
	}
	
	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = INADDR_ANY;
	_address.sin_port = htons(_port);

	Logger::debug("Binding socket");
	if (bind(_server_fd, (sockaddr *)&_address, sizeof(_address)) == -1)
	{
		Logger::error("Bind failed");
		throw std::runtime_error("failed to setup the server");
	}

	Logger::debug("Listening on socket");
	if (listen(_server_fd, 10) == -1)
	{
		Logger::error("Listen failed");
		throw std::runtime_error("failed to setup the server");
	}

	std::cout << "Server listening on port " << _port << "...\n";
	isrunning = true;
	return true;
}

int Socket::acceptClient()
{
	int fd = accept(_server_fd, (sockaddr *)&_address, &_addrlen); 
	return fd;
}

int Socket::response(const Client& client)
{
	return(send(client.getClientFd(), this->res.c_str(), this->res.length(), 0));
}

int Socket::getServerFd() const
{
	return _server_fd;
}

void Socket::setResponse(std::string res)
{
	this->res = res;
}
