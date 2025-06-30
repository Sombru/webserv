#pragma once
#include "Webserv.hpp"
#include "Socket.hpp"
#include "ParserManager.hpp"

class ServerManager {
private:
    //ServerConfig serverConfig;
	std::vector<pollfd> pollfds;
	std::vector<Client> clients;
    Socket serverSocket;
public:
    //ServerManager(const ServerConfig& config)
    //    : serverConfig(config), serverSocket(config.port) {}
	ServerManager(const Socket& webserv);
    void run();
	void createClient();
	void handleClients(size_t clientIndex);
};

