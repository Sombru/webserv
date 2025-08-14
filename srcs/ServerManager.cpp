#include "ServerManager.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>


ServerManager::ServerManager(FullConfig &configSrc)
	: config(configSrc), epoll_fd(-1), runnig(false)
{
	for (size_t i = 0; i < config.servers.size(); i++)
	{
		sockets.push_back(config.servers[i]);
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
	events.reserve(sockets.size() + 1);
	for (size_t i = 0; i < sockets.size(); ++i)
	{
		if (sockets[i].setup() < 0)
			return -1;
		epoll_event sock_event;
		sock_event.events = EPOLLIN;
		sock_event.data.fd = sockets[i].fd;
		events.push_back(sock_event);
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockets[i].fd, &events[i]) < 0)
		{
			ERROR("Failed to add " + config.servers[i].name + " to epoll: "+ errstr);
			return -1;
		}
		int flags = fcntl(sockets[i].fd, F_GETFL, 0); // GETFLAGS (GETFLAGS NOT ALLOWED)
		if (flags < 0)
		{
			ERROR("Failed to get socket flags: " + errstr);
			return -1;
		}

		if (fcntl(sockets[i].fd, F_SETFL, flags | O_NONBLOCK) < 0) // SETFLAGS of this fd to nonblocking
		{
			ERROR("Failed to set non-blocking: " + errstr);
			return -1;
		}
		biggest_fd = sockets[i].fd;
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
			int fd = events[i].data.fd;

			if (fd <= biggest_fd)
			{
				acceptConnection(fd);
			}
			else 
			{
				handleClientData(fd);
			}
		}	
		// cleanupTimeouts();
	}	
}

void ServerManager::acceptConnection(int &fd)
{
	int target = -1;

	for (size_t i = 0; i < sockets.size(); ++i)
	{
		if (sockets[i].fd == fd)
			target = i;
	}
	
	if (target == -1)
		return;

	while (true)
	{
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);

		int client_fd = accept(sockets[target].fd, (struct sockaddr *)&client_addr, &client_len);
		if (client_fd < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				break; // No more connections
			}
			ERROR("Accept failed: " + errstr);
			break;
		}

		if (fcntl(client_fd, F_SETFL, 0 | O_NONBLOCK) < 0)
		{
			std::cerr << "Failed to set non-blocking: " << strerror(errno) << std::endl;
			close(client_fd);
			return ;
		}

		struct epoll_event event;
		event.events = EPOLLIN | EPOLLET;
		event.data.fd = client_fd;
		
		if ((epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0))
		{
			ERROR("Failed to add clinet to epoll: " + errstr);
			close(client_fd);
			return;
		}

		char clinetIP[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &client_addr.sin_addr, clinetIP, INET_ADDRSTRLEN);
		INFO("New clinet " + intToString(client_fd) + " connected");
	}

	
}


void ServerManager::handleClientData(int &fd)
{
	char buffer[4096];

	while (true)
	{
		ssize_t bytesRead = recv(fd, buffer, sizeof(buffer), 0);
		if (bytesRead < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				break; // No more data
			}
		}

		if (bytesRead == 0)
		{
			INFO("Client " + intToString(fd) + " disconected");
			close(fd);
			return;
		}
		send(fd, "TESTO", 6, 0);
	}
	
}