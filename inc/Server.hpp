#pragma once

class Client;
// class HTTP;

#include "HTTP.hpp"
#include "Webserv.hpp"
#include "Logger.hpp"
#include "Client.hpp"


#define MAX_CONNECTIONS SOMAXCONN // maximum conections to listen to 

class Server
{
private:
	int port;
	std::string addresStr;
	ServerConfig &serverConfig;
	HTTP http;

public:
	int server_fd;

	// Server();
	Server(ServerConfig &serverSrc);
	Server(const Server &other); 
	Server &operator=(const Server &other);
	int setup();

	bool setNonBlocking(int fd);
	void acceptConnection(int &epoll_fd, std::map<int, Client> &clientsMap);
	bool handleConnection(int fd);  // Return false if client should be removed
	bool sendResponse(int fd, const std::string &response);
	bool sendResponse(int fd, const HttpResponse &response);

	~Server();
};
