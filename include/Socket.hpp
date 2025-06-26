#pragma once

#include "Webserv.hpp"
#include "Client.hpp"

#define PORT 8080

class Client;

class Socket
{
private:
	int _server_fd;
	int _port;
	struct sockaddr_in _address;
	socklen_t _addrlen;

	std::string res;

public:
	Socket() {};
	Socket(int port);
	~Socket();

	bool isrunning;

	bool setup();
	int acceptClient();
	int response(const Client& client);

	int getServerFd() const;
	void setResponse(std::string res);
};