#pragma once

#include "Logger.hpp"
#include "Webserv.hpp"
#include "Utils.hpp"
#include "Socket.hpp"

class ServerManager
{
private:
	std::vector<ServerConfig> &config;
	std::vector<Socket> sockets;
	int epoll_fd;
	int cleanupInterval; // an intervall to check if any connection is timed out (in seconds)

public:
	bool runnig;
	ServerManager(std::vector<ServerConfig> &configSrc);
	int setup();
	~ServerManager();
};
