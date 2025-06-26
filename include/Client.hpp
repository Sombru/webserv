#pragma once

#include "Webserv.hpp"
#include "Socket.hpp"

class Client
{
private:
	int _fd;
	std::string raw_request;

public:
	Client(Socket& server);
	~Client();

	int getClientFd() const;
	std::string getRawRequest() const;

	ssize_t sendMessage(std::string response);
	void recievedRequest();

};
 ;