#include "Socket.hpp"
#include "Logger.hpp"

Socket::Socket(const ServerConfig &server)
{
	addresStr = (server.host.substr(0, server.host.find(':')));
	port = atoi(server.host.substr(server.host.find(':') + 1).c_str());
}

int Socket::setup()
{
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		ERROR("Could not create socket: " + strerror(errno));
	}

	if ()
}

bool setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
	{
		ERROR("Failed to get socket flags: " + strerror(errno));
		return false;
	}

	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		ERROR("Failed to set non-blocking: " + strerror(errno));
		return false;
	}

	return true;
}