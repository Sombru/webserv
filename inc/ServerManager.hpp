#pragma once

#include "Logger.hpp"
#include "Webserv.hpp"
#include "Utils.hpp"
#include "Server.hpp"
#include "Client.hpp"

class ServerManager
{
private:
	FullConfig &config;
	std::vector<Server> servers;

	std::map<int, Server> serversMap;
	std::map<int, Client> clientsMap;
	// std::map<int, time_t> clientActivity;
	int epoll_fd;
	time_t lastTimeoutCheck;
	static volatile sig_atomic_t signalReceived;

public:
	bool running;
	ServerManager(FullConfig &configSrc);
	int setup();
	void run();
	void checkTimeouts();
	void updateClientActivity(int client_fd);
	void removeClient(int client_fd);
	static void signalHandler(int signal);

	~ServerManager() {};
};
