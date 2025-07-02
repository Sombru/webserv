#pragma once
#include "Webserv.hpp"
#include "Socket.hpp"
#include "ParserManager.hpp"

class ServerManager {
private:
    //ServerConfig serverConfig;
	int epoll_fd;
	std::map<int, Client> clients;
    Socket serverSocket;
public:
	bool running;
    //ServerManager(const ServerConfig& config)
    //    : serverConfig(config), serverSocket(config.port) {}
	ServerManager(const Socket& webserv);
    void run();
	void createClient();
	void handleClient(Client& client);
	void setupEpoll(int fd);
	void addClientToEpoll(int epoll_fd, int fd);
	void shutdown_epoll();
	void stop(); // stops the loop
};

