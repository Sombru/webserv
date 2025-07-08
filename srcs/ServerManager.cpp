#include "Webserv.hpp"
#include "Config.hpp"
#include "ServerManager.hpp"
#include "Logger.hpp"
#include "Client.hpp"

// ServerManager::ServerManager(ServerConfig &serv)
// 	: server(serv), socket(this->server)
// {
// 	socket.setup();
// 	int fd = socket.getServerFD();
// 	set_non_blocking(fd);
// 	setupEpoll(fd);
// 	running = true; // initializing the running of the while loop
// 	Logger::debug("[Server socket][" + intToString(fd) + "] initialized with epoll");
// }

// ServerManager::~ServerManager()
// {

// }

// void ServerManager::setupEpoll(int fd)
// {
// 	epoll_fd = epoll_create1(0);
// 	if (epoll_fd == -1)
// 		throw std::runtime_error("Failed to create epoll instance");

// 	epoll_event ev;
// 	ev.events = EPOLLIN;
// 	ev.data.fd = fd;

// 	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1)
// 		throw std::runtime_error("Failed to add server socket to epoll");
// }

// void ServerManager::addClientToEpoll(int epoll_fd, int fd)
// {
//     epoll_event ev;
//     ev.events = EPOLLIN | EPOLLET;
//     ev.data.fd = fd;

//     if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) 
//     {
//         close(fd);
//         throw std::runtime_error("Failed to add fd " + intToString(fd) + " to epoll");
//     }
// }

// void ServerManager::createClient()
// {
//     try 
//     {
//         Client client(this->socket);
//         int client_fd = client.getClientFd();
//         set_non_blocking(client_fd);
//         addClientToEpoll(epoll_fd, client_fd);
//         clients.insert(std::make_pair(client_fd, client));
//         Logger::info("[" + intToString(client_fd) + "] New client connected");
//     }
//     catch (const std::exception& e) 
//     {
//         Logger::error(std::string("Failed to create client: ") + e.what());
//     }
// }

// /**
//  * @brief Handles the client request-response cycle.
//  *        If any error occurs, removes client from epoll and client map, and closes socket.
//  * @param client - Reference to the client object to handle.
//  */
// void ServerManager::handleClient(Client& client)
// {
//     // int fd = client.getClientFd();
//     // try
//     // {
//     //     client.getRequest();
//     //     CommunicationManager pm;
//     //     pm.parseRequest(client);
//     //     pm.buildResponse(serverSocket, client);
//     //     Logger::info("[" + intToString(fd) + "] request and response completed");
//     // }
//     // catch (const std::exception& e)
//     // {
//     //     Logger::error("Client [" + intToString(fd) + "] handling failed: " + e.what());
//     //     if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1)
//     //         perror("epoll_ctl DEL");
//     //     close(fd);
//     //     clients.erase(fd);
//     //     Logger::info("Closed and removed client fd " + intToString(fd));
//     // }
// 	std::cout << client.getRaw_request();
// }

// void ServerManager::serverLoop()
// {
	
// }

// ServerConfig &ServerManager::getServer() const
// {
// 	return this->server;
// }



ServerManager::ServerManager(const std::vector<ServerConfig>& serverConfigs) //🍓
: servers(serverConfigs), running(true) {
    epoll_fd = epoll_create(0);
    if (epoll_fd == -1)
        throw std::runtime_error("Failed to create epoll instance");
    
    initServerSockets();
}

void ServerManager::initServerSockets() { //🍓
     std::set<std::string> seen;

    for (size_t i = 0; i < servers.size(); ++i) {
        const ServerConfig& config = servers[i];

        std::string key = config.host + ":" + intToString(config.port);
        if (!seen.insert(key).second)
            throw std::runtime_error("Duplicate bind attempt: " + key);
        
        Socket s(config);
        s.setup();

        int fd = s.getServerFd();
        set_non_blocking(fd);
        addFdToEpoll(fd);

        serverSockets[fd] = config;
        Logger::debug("[Server Socket][" + intToString(fd) + "] initialized with epoll");
    }
}

void ServerManager::serverLoop() { //🍓
    const int maxEvents = 100;
    struct epoll_event events[maxEvents];

    while (running) {
        int nbrReadyEventsreturned = epoll_wait(epoll_fd, events, maxEvents, -1);
        if (nbrReadyEventsreturned < 0)
            throw std::runtime_error("epoll_wait failed");

        for (int i = 0; i < nbrReadyEventsreturned; ++i) {
            int fd = events[i].data.fd; // each epoll_event == events, contains the fd that triggered the event

            std::cout << fd << std::endl;
            if (serverSockets.count(fd)) { // if fd is a server socket, waits for a new connection 
                acceptNewClient(fd);
            } else if (clients.count(fd)) { // if fd is a client socket, is ready to read data from a client
                handleClient(fd);
            }
        }
    }
}

void ServerManager::acceptNewClient(int server_fd) {
    int client_fd = accept(server_fd, NULL, NULL); // accepts a new connection on the listening socket server_fd
    if (client_fd == -1)
    {
        Logger::error("accept() failed");
        return ;
    }

    set_non_blocking(client_fd);
    addFdToEpoll(client_fd);

    // Associat the client with its server config
    Client client(serverSockets[server_fd]);
    client.setClientFd(client_fd);

    // Store in clients map
    clients[client_fd] = client;

    Logger::info("[" + intToString(client_fd) + "] New client connected");
}

void ServerManager::handleClient(int fd) {
    Client& client = clients[fd];
    try {
        client.receiveRequest();
        // parse + response here

        // for now, print the request
        std::cout << client.getRaw_request() << std::endl;
    }
    catch (...) {
        Logger::info("Closing client " + intToString(fd));
        close(fd);
        clients.erase(fd);
    }
}

void ServerManager::set_non_blocking(int fd) //🍓
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
        Logger::info("Failed to set " + intToString(fd) + " to non-blocking mode");
	}
	Logger::info("Set fd " + intToString(fd) + " to non-blocking mode");
}

void ServerManager::addFdToEpoll(int fd) { //🍓
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1)
        throw std::runtime_error("Failed to add fd to epoll");

    Logger::info("added to epoll()");
}