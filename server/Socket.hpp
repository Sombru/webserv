#pragma once

#include "libraries.hpp"

#define PORT 8080

class Socket
{
private:
    int _server_fd;
    int _port;
    struct sockaddr_in _address;
    socklen_t _addrlen;

public:
	Socket();
	Socket(int port);
	~Socket();

	bool setup();
    int acceptClient();
    int getServerFd() const;
};