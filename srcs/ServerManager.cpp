#include "ServerManager.hpp"

ServerManager::ServerManager(std::vector<ServerConfig> &configSrc)
	: config(configSrc), epoll_fd(-1), cleanupInterval(5), runnig(false)
{
	for (size_t i = 0; i < config.size(); i++)
	{
		sockets.push_back(config[i]);
	}
}

int ServerManager::setup()
{


	for (size_t i = 0; i < config.size(); i++)
	{
		
	}
	
}