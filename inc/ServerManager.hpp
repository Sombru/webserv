#pragma once

#include "Webserv.hpp"
#include "Config.hpp"
#include "Socket.hpp"

class Socket;
class Client;

class ServerManager
{
private:
	const std::vector<ServerConfig>& servers;
	// int epoll_fd;
	// std::map<int, Client> clients;
	std::vector<Socket> sockets;
public:
	bool running;

	ServerManager(const std::vector<ServerConfig>& servers);
	~ServerManager();

	void setup();
	void run();

};

