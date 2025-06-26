#pragma once

#include "Webserv.hpp"
#include "Socket.hpp"

class Socket;

class Client
{
private:
	int _fd;
	std::string raw_request;
	
public:
	Client() {} ;
	Client(Socket& webserv);
	~Client();

	int getClientFd() const;
	std::string getRaw_request() const;
	
	ssize_t response(std::string response);
	void getRequest();
	// HttpRequest parseRequest();

};
