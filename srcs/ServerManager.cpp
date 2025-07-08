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
	while (running)
	{
		for (size_t i = 0; i < sockets.size(); ++i)
		{
			Client client(sockets[i]);
			client.getRequest();
			Logger::debug(client.getRaw_request());
		}
	}
}

ServerManager::~ServerManager()
{
	Logger::info("Shutting down webserver");
}
