#include "Socket.hpp"
#include "Logger.hpp"

Socket::Socket(const ServerConfig &server)
{
	addresStr = (server.host.substr(0, server.host.find(':')));
	port = atoi(server.host.substr(server.host.find(':') + 1).c_str());
	name = server.name;
}

Socket::Socket() {}

Socket::~Socket() {}

int Socket::setup()
{
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		ERROR("Could not create socket: " + errstr);
		return -1;
	}
	int opt = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		ERROR("Failed to set socket options for " + name + ": " + errstr);
		close(fd);
		return false;
	}

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo *res;
	if (getaddrinfo(addresStr.c_str(), intToString(port).c_str(), &hints, &res) < 0)
	{
		ERROR("Failed to get addres info for " + name + ": " + errstr);
		return -1;
	}

	if (bind(fd, res->ai_addr, res->ai_addrlen) < 0)
	{
		freeaddrinfo(res);
		ERROR("Failed to bind socket for " + name + ": " + errstr);
		return -1;
	}
	freeaddrinfo(res); // free result after use

	if (listen(fd, MAX_CONNECTIONS) < 0)
	{
		ERROR("Failed to listen for connection for " + name + ": " + errstr);
		return -1;
	}
	INFO("Server '" + name + "' listenig on address: " + addresStr);
	return EXIT_SUCCESS;
}

// bool setNonBlocking(int fd)
// {
// 	int flags = fcntl(fd, F_GETFL, 0); // GETFLAGS
// 	if (flags < 0)
// 	{
// 		ERROR("Failed to get socket flags: " + errstr);
// 		return false;
// 	}

// 	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) // SETFLAGS of this fd to nonblocking
// 	{
// 		ERROR("Failed to set non-blocking: " + errstr);
// 		return false;
// 	}
// 	return true;
// }