#pragma once

#include "Webserv.hpp"
#include "Config.hpp"
#include "Socket.hpp"

class Socket;
class Client;

class ServerManager
{
private:
	struct ServerConfig& server;
	int epoll_fd;
	std::map<int, Client> clients;
	Socket socket;

	void set_non_blocking(int fd);
	void setupEpoll(int fd);
	void handleClient(Client& client);
	void addClientToEpoll(int epoll_fd, int fd);
	
public:

	bool running;

	ServerManager(ServerConfig& serv);
	~ServerManager();

	void serverLoop();


	void createClient();

	ServerConfig& getServer() const;
};

