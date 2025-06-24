#pragma once

#include "Libraries.hpp"
#include "Socket.hpp"

class Client
{
private:
	int _fd;

public:
	Client(Socket& server);
	~Client();

	int getClientFd() const;
	

	ssize_t sendMessage(std::string response);
	std::string recievedRequest();

};
