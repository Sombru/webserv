#pragma once

#include "Webserv.hpp"
#include "Logger.hpp"

#define MAX_CONNECTIONS SOMAXCONN // maximum conections to listen to 

// Forward declaration to avoid circular dependency
struct ClientInfo;

class Server
{
private:
	int port;
	std::string addresStr;
	ServerConfig serverConfig;  // Changed from reference to copy

public:
	int server_fd;

	Server();  // Add default constructor
	Server(ServerConfig &serverSrc);
	Server(const Server& other);  // Add copy constructor
	Server& operator=(const Server& other);  // Add assignment operator
	int setup();

	bool setNonBlocking(int fd);
	void acceptConnection(int &epoll_fd, std::map<int, ClientInfo> &clientsMap);
	bool handleConnection(int fd);  // Return false if client should be removed

	~Server();
};
