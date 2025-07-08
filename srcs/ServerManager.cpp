#include "ServerManager.hpp"
#include "Logger.hpp"
#include "Client.hpp"

ServerManager::ServerManager(const std::vector<ServerConfig>& servers)
: servers(servers), running(true)
{
	this->sockets.reserve(this->servers.size()); // reserve just enought space for every socket

	for (unsigned int i = 0; i < this->servers.size(); i++)
	{
		this->sockets.push_back(Socket(this->servers[i])); // init every socket for every server
	}
}

void ServerManager::setup()
{
	for (unsigned int i = 0; i < this->sockets.size(); i++)
		this->sockets[i].setup();
}

void ServerManager::run()
{
	Client client(this->sockets[0]);
	Client client2(this->sockets[1]);
	client.getRequest();
	client2.getRequest();
	Logger::debug(client.getRaw_request());
	Logger::debug(client2.getRaw_request());
}

ServerManager::~ServerManager()
{
	Logger::info("Shutting down webserver");
}
