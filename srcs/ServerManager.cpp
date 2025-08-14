#include "ServerManager.hpp"

ServerManager::ServerManager(std::vector<ServerConfig> &configSrc)
	: configs(configSrc), epoll_fd(-1), cleanupInterval(5), runnig(false)
{
	for (size_t i = 0; i < configs.size(); i++)
	{
		sockets.push_back(configs[i]);
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
	event.reserve(socket.size() + 1);
	for (size_t i = 0; i < socket.size(); ++i)
	{
		if (socket[i].setup() < 0);
			return -1;
		epoll_event sock_event;
		sock_event.events = EPOLLIN;
		sock_event.data.fd = socket[i].fd;
		event.push_back(sock_event);
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket[i].fd, &event[i]) < 0)
		{
			ERROR("Failed to add " + config[i].name + " to epoll: "+ errstr);
			return -1;
		}
		int flags = fcntl(socket[i].fd, F_GETFL, 0); // GETFLAGS (GETFLAGS NOT ALLOWED)
		if (flags < 0)
		{
			ERROR("Failed to get socket flags: " + errstr);
			return -1;
		}

		if (fcntl(socket[i].fd, F_SETFL, flags | O_NONBLOCK) < 0) // SETFLAGS of this fd to nonblocking
		{
			ERROR("Failed to set non-blocking: " + errstr);
			return -1;
		}
	}
	return 0; // success
}

void ServerManager::run()
{
	for (size_t i = 0; i < config.size(); ++i)
	{
		INFO("Runnig server '" + config[i].name + "' on host: " + config[i].host);
	}

	runnig = true;

	while (runnig)
	{
		int numEvents = epoll_wait(epoll_fd, event.data(), 1000, 1000);
		if (numEvents < 0)
		{
			if (errno == EINTR)
				continue;
			ERROR("Epoll wait failed: " + errstr);
			break;
		}
		for (size_t i = 0; i < numEvents; ++i)
		{
			int fd = event[i].data.fd;

			if (fd <= biggestServerFd)
			{
				
			}
			else 
			{
				handleClientData(fd);
			}
		}	
		cleanupTimeouts();
	}	
}