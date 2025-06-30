#include "ServerManager.hpp"

ServerManager::ServerManager(const Socket& webserv): serverSocket(webserv)
{
    serverSocket.setup();
	pollfd serverPollFd = {serverSocket.getServerFd(), POLLIN, 0};
    pollfds.push_back(serverPollFd);
}

void ServerManager::createClient()
{
	Client client(serverSocket);
	pollfd clientPollFd = {client.getClientFd(), POLLIN, 0 };
	pollfds.push_back(clientPollFd);
	clients.push_back(client);
}

void ServerManager::handleClient(size_t clientIndex)
{
    clients[clientIndex].getRequest();
    ParserManager pm;
    pm.parseRequest(clients[clientIndex]);
    pm.buildResponse(serverSocket, clients[clientIndex]);
    serverSocket.response(clients[clientIndex]);
}

void ServerManager::run()
{
	while (true)
	{
		int ret = poll(pollfds.data(), pollfds.size(), -1);
		if (ret < 0) {
			perror("poll");
			break;
		}
		if (pollfds[0].revents & POLLIN)
			createClient();

		for (size_t i = 1; i < pollfds.size(); ++i)
		{
			if (pollfds[i].revents & POLLIN)
				handleClient(i - 1);
		}
	}
}

