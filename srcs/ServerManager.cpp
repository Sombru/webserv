#include "ServerManager.hpp"
#include "Logger.hpp"
#include "Client.hpp"
#include "HTTP.hpp"

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
	std::vector<pollfd> poll_fds;

	// Setup pollfd list from all server sockets
	for (size_t i = 0; i < sockets.size(); ++i)
	{
		pollfd pfd;
		pfd.fd = sockets[i].getServerFD();
		pfd.events = POLLIN; // wait for incoming connections
		poll_fds.push_back(pfd);
	}

	while (running)
	{
		int ret = poll(&poll_fds[0], poll_fds.size(), -1);
		if (ret < 0)
		{
			perror("poll");
			break;
		}

		for (size_t i = 0; i < poll_fds.size(); ++i)
		{
			if (poll_fds[i].revents & POLLIN)
			{
				Client client(poll_fds[i].fd, this->sockets[i].getRequestSize());
				client.makeRequest();
				HttpRequest request = parseRequest(client.getRaw_request(), this->servers[i]);
				// generateResponse(request);
				Logger::debug(request);
				HttpResponse response = recieveResponse(request, this->servers[i]);
			}
		}
	}
}

ServerManager::~ServerManager()
{
	Logger::info("Shutting down webserver");
}
