#include "ServerManager.hpp"


ServerManager::ServerManager(FullConfig &configSrc)
	: config(configSrc), epoll_fd(-1), lastTimeoutCheck(time(NULL)), runnig(false)
{
	servers.reserve(config.servers.size());
	for (size_t i = 0; i < config.servers.size(); i++)
	{
		servers.push_back(Server(config.servers[i]));
	}
}

int ServerManager::setup()
{
	epoll_fd = epoll_create1(0);
	if (epoll_fd < 0)
	{
		ERROR("Failed to create epoll: " + errstr);
		return -1;
	}
	events.reserve(servers.size() + 1);
	for (size_t i = 0; i < servers.size(); ++i)
	{
		if (servers[i].setup() < 0)
			return -1;
		
		serversMap[servers[i].server_fd] = servers[i];
		
		epoll_event sock_event;
		sock_event.events = EPOLLIN;
		sock_event.data.fd = servers[i].server_fd;
		events.push_back(sock_event);
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, servers[i].server_fd, &events[i]) < 0)
		{
			ERROR("Failed to add " + config.servers[i].name + " to epoll: "+ errstr);
			return -1;
		}
	}
	return 0; // success
}

void ServerManager::run()
{
	for (size_t i = 0; i < config.servers.size(); ++i)
	{
		INFO("Runnig server '" + config.servers[i].name + "' on host: " + config.servers[i].host);
	}

	runnig = true;

	while (runnig)
	{
		int numEvents = epoll_wait(epoll_fd, events.data(), config.maxEvents, config.timeout);

		if (numEvents < 0)
		{
			if (errno == EINTR)
				continue;
			ERROR("Epoll wait failed: " + errstr);
			break;
		}

		for (int i = 0; i < numEvents; ++i)
		{
			int event_fd = events[i].data.fd;

			// Check if this is a server socket (new connection)
			if (serversMap.find(event_fd) != serversMap.end())
			{
				serversMap[event_fd].acceptConnection(epoll_fd, clientsMap);
			}
			// Otherwise, it's a client socket (existing connection)
			else if (clientsMap.find(event_fd) != clientsMap.end())
			{
				// updateClientActivity(event_fd);  // Update activity timestamp
				clientsMap[event_fd].server.handleConnection(event_fd);
				// removeClient(event_fd); if no keep-alive header = close connection immidietly
			}
			else
			{
				WARNING("Unknown file descriptor in epoll event: " + intToString(event_fd));
			}
		}
		WARNING("Someone should be disconected");
		// Check for timeouts every few seconds (avoid checking too frequently)
		time_t currentTime = time(NULL);
		if (currentTime - lastTimeoutCheck >= 5)  // Check every 5 seconds
		{
			cleanupTimeouts();
			lastTimeoutCheck = currentTime;
		}
	}	
}

void ServerManager::updateClientActivity(int client_fd)
{
	if (clientsMap.find(client_fd) != clientsMap.end())
	{
		clientsMap[client_fd].lastActivity = time(NULL);
	}
}

void ServerManager::removeClient(int client_fd)
{
	if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL) < 0)
	{
		WARNING("Failed to remove client " + intToString(client_fd) + " from epoll: " + errstr);
	}
	
	close(client_fd);
	
	clientsMap.erase(client_fd);
	
	INFO("Client " + intToString(client_fd) + " removed due to timeout or disconnect");
}

void ServerManager::cleanupTimeouts()
{
	time_t currentTime = time(NULL);
	std::vector<int> clientsToRemove;
	
	// Collect clients that have timed out
	for (std::map<int, ClientInfo>::iterator it = clientsMap.begin(); 
		 it != clientsMap.end(); ++it)
	{
		int client_fd = it->first;
		time_t lastActivity = it->second.lastActivity;
		
		// Check if client has been inactive for more than timeout seconds
		// config.timeout is in milliseconds, so convert to seconds
		int timeoutSeconds = (config.timeout > 0) ? config.timeout / 1000 : 30; // Default 30 seconds
		
		if (currentTime - lastActivity > timeoutSeconds)
		{
			clientsToRemove.push_back(client_fd);
		}
	}
	
	// Remove timed out clients
	for (std::vector<int>::iterator it = clientsToRemove.begin(); 
		 it != clientsToRemove.end(); ++it)
	{
		INFO("Client " + intToString(*it) + " timed out after " + 
			 intToString(currentTime - clientsMap[*it].lastActivity) + " seconds");
		removeClient(*it);
	}
	
	if (!clientsToRemove.empty())
	{
		INFO("Cleaned up " + intToString(clientsToRemove.size()) + " timed out connections");
	}
}