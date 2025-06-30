#include "ServerManager.hpp"
#include "Logger.hpp"

ServerManager::ServerManager(const Socket& webserv) : serverSocket(webserv)
{
    serverSocket.setup();
    int fd = serverSocket.getServerFd();
    pollfd serverPollFd = {fd, POLLIN, 0};
    pollfds.push_back(serverPollFd);
    Logger::debug("Server socket initialized on fd = " + std::to_string(fd));
}

void ServerManager::createClient()
{
	try 
    {
		Client client(serverSocket);
		pollfd clientPollFd = {client.getClientFd(), POLLIN, 0 };
		pollfds.push_back(clientPollFd);
		clients.push_back(client);
		Logger::info("New client connected with fd = " + std::to_string(client.getClientFd()));
	} 
    catch (const std::exception& e) 
    {
		Logger::error(std::string("Failed to create client: ") + e.what());
	}
}

void ServerManager::handleClients(size_t clientIndex)
{
    try 
    {
        clients[clientIndex].getRequest();
        ParserManager pm;
        pm.parseRequest(clients[clientIndex]);
        pm.buildResponse(serverSocket, clients[clientIndex]);
        //serverSocket.response(clients[clientIndex]);
    }
    catch (const std::exception& e) 
    {
        Logger::error("Client [" + std::to_string(clients[clientIndex].getClientFd()) + "] request handling failed: " + e.what());
        close(clients[clientIndex].getClientFd());
    }
}

void ServerManager::run()
{
	while (true)
	{
		int ret = poll(pollfds.data(), pollfds.size(), -1);
		if (ret < 0) 
        {
			perror("poll");
			throw std::runtime_error("poll() failed");
		}
		Logger::debug("poll() succeeded, " + std::to_string(ret) + " file descriptor(s) ready for I/O");
		if (pollfds[0].revents & POLLIN) 
        {
			Logger::debug("Incoming connection detected on server socket (FD = " + std::to_string(pollfds[0].fd) + "), accepting client...");
			createClient();
		}

        for (size_t i = 1; i < pollfds.size(); ++i)
        {
            if (pollfds[i].revents & POLLIN)
            {
                Logger::debug("Data ready on client fd: " + std::to_string(pollfds[i].fd));
                handleClients(i - 1);
                pollfds[i].revents = 0; // Reset after handling to avoid duplicate processing
            }
        }
	}
}

