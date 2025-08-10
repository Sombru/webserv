#pragma once

#include "Webserv.hpp"
#include "Logger.hpp"

class Socket
{
private:
	int fd;
	int port;
	std::string addresStr;
	struct addrinfo address;
	socklen_t addrlen;
	int bufferSize;

public:
	Socket(const ServerConfig &server);	
	int setup();
	~Socket();
};
