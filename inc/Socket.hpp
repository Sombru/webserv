#pragma once

#include "Webserv.hpp"
#include "Logger.hpp"

#define MAX_CONNECTIONS SOMAXCONN // maximum conections to listen to 

class Socket
{
private:
	int port;
	std::string addresStr;
	struct addrinfo address;
	socklen_t addrlen;
	int bufferSize;

	std::string name;

public:

	int fd;

	Socket();
	Socket(const ServerConfig &server);
	int setup();
	~Socket();
};
