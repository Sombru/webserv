#pragma once

#include "Logger.hpp"
#include "Webserv.hpp"
#include "Utils.hpp"
#include "Socket.hpp"

class ServerManager
{
private:
	std::vector<ServerConfig> &configs;
	std::map<Socket, int> sockets;
	std::vector<epoll_event> events;
	int epoll_fd;
	// int cleanupInterval; // an intervall to check if any connection is timed out (in seconds)


public:
	bool runnig;
	ServerManager(std::vector<ServerConfig> &configSrc);
	int setup();
	void run();
	~ServerManager();
};
