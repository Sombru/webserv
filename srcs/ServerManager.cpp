#include "Webserv.hpp"
#include "Config.hpp"
#include "ServerManager.hpp"
#include "Logger.hpp"
#include "Client.hpp"

ServerManager::ServerManager(ServerConfig &serv)
	: server(serv), socket(this->server)
{
	socket.setup();
	int fd = socket.getServerFD();
	set_non_blocking(fd);
	setupEpoll(fd);
	running = true; // initializing the running of the while loop
	Logger::debug("[Server socket][" + intToString(fd) + "] initialized with epoll");
}

ServerManager::~ServerManager()
{

}

void ServerManager::set_non_blocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
	{
		Logger::error("Failed to get flags for fd " + intToString(fd));
		throw std::runtime_error("fcntl F_GETFL failed");
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		Logger::error("Failed to set O_NONBLOCK for fd " + intToString(fd));
		throw std::runtime_error("fcntl F_SETFL failed");
	}
	Logger::info("Set fd " + intToString(fd) + " to non-blocking mode");
}

void ServerManager::setupEpoll(int fd)
{
	epoll_fd = epoll_create1(0);
	if (epoll_fd == -1)
		throw std::runtime_error("Failed to create epoll instance");

	epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = fd;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1)
		throw std::runtime_error("Failed to add server socket to epoll");
}

void ServerManager::addClientToEpoll(int epoll_fd, int fd)
{
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) 
    {
        close(fd);
        throw std::runtime_error("Failed to add fd " + intToString(fd) + " to epoll");
    }
}

void ServerManager::createClient()
{
    try 
    {
        Client client(this->socket);
        int client_fd = client.getClientFd();
        set_non_blocking(client_fd);
        addClientToEpoll(epoll_fd, client_fd);
        clients.insert(std::make_pair(client_fd, client));
        Logger::info("[" + intToString(client_fd) + "] New client connected");
    }
    catch (const std::exception& e) 
    {
        Logger::error(std::string("Failed to create client: ") + e.what());
    }
}

/**
 * @brief Handles the client request-response cycle.
 *        If any error occurs, removes client from epoll and client map, and closes socket.
 * @param client - Reference to the client object to handle.
 */
void ServerManager::handleClient(Client& client)
{
    // int fd = client.getClientFd();
    // try
    // {
    //     client.getRequest();
    //     CommunicationManager pm;
    //     pm.parseRequest(client);
    //     pm.buildResponse(serverSocket, client);
    //     Logger::info("[" + intToString(fd) + "] request and response completed");
    // }
    // catch (const std::exception& e)
    // {
    //     Logger::error("Client [" + intToString(fd) + "] handling failed: " + e.what());
    //     if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1)
    //         perror("epoll_ctl DEL");
    //     close(fd);
    //     clients.erase(fd);
    //     Logger::info("Closed and removed client fd " + intToString(fd));
    // }
	std::cout << client.getRaw_request();
}

void ServerManager::serverLoop()
{
	
}

ServerConfig &ServerManager::getServer() const
{
	return this->server;
}
