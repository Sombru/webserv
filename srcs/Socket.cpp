#include "include/Socket.hpp"

Socket::Socket(int port) 
: _server_fd(-1), _port(port), _addrlen(sizeof(_address))
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
	_server_fd = socket(AF_INET, SOCK_STREAM, 0); // returns a file descriptor that refers to that endpoint(just like open())
	if (_server_fd < 0)
	{
		std::cerr << "Socket creation failed\n";
		return false;
	}

	int opt = 1;
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		std::cerr << "setsockopt failed\n";
		return false;
	}

	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = INADDR_ANY;
	_address.sin_port = htons(_port);

	if (bind(_server_fd, (struct sockaddr *)&_address, sizeof(_address)) == -1)
	{
		std::cerr << "Bind failed\n";
		return false;
	}

	if (listen(_server_fd, 10) == -1)
	{
		std::cerr << "Listen failed\n";
		return false;
	}

	std::cout << "Server listening on port " << _port << "...\n";
	return true;
}

int Socket::acceptClient()
{
	return accept(_server_fd, (struct sockaddr *)&_address, &_addrlen);
}

int Socket::getServerFd() const
{
	return _server_fd;
}
