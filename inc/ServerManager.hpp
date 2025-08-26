#pragma once

#include "Logger.hpp"
#include "Webserv.hpp"
#include "Utils.hpp"
#include "Server.hpp"
#include <ctime>

struct ClientInfo 
{
	Server server;
	time_t lastActivity;
	
	ClientInfo() : lastActivity(time(NULL)) {}
	ClientInfo(const Server& srv) : server(srv), lastActivity(time(NULL)) {}
};

class ServerManager
{
private:
	FullConfig &config;
	std::vector<Server> servers;
	std::map<int, Server> serversMap; 
	std::map<int, ClientInfo> clientsMap;  // Changed from Server to ClientInfo
	std::vector<epoll_event> events;
	int epoll_fd;
	time_t lastTimeoutCheck;

public:
	bool runnig;
	ServerManager(FullConfig &configSrc);
	int setup();
	void run();
	void cleanupTimeouts();
	void updateClientActivity(int client_fd);
	void removeClient(int client_fd);

	~ServerManager() {};
};
