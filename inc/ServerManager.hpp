#pragma once

#include "Logger.hpp"
#include "Webserv.hpp"
#include "Utils.hpp"
#include "Socket.hpp"

class ServerManager
{
private:
	FullConfig &config;
	std::vector<Socket> sockets;
	std::vector<epoll_event> events;
	int epoll_fd;
	// int cleanupInterval; // an intervall to check if any connection is timed out (in seconds)

	int biggest_fd;

public:
	bool runnig;
	ServerManager(FullConfig &configSrc);
	int setup();
	void run();
	void acceptConnection(int &fd);
	void handleClientData(int &fd);

	~ServerManager() {};

};
