#include "ServerManager.hpp"
#include "Logger.hpp"

/**
 * @brief Constructs ServerManager, sets up the server socket, sets it non-blocking,
 *        and initializes epoll to monitor the server socket.
 * @param webserv - Socket object representing the server socket.
 */
ServerManager::ServerManager(const Socket& webserv) : serverSocket(webserv)
{
    serverSocket.setup();
    int fd = serverSocket.getServerFd();
    set_non_blocking(fd);
    setupEpoll(fd);
    running = true; // initializing the running of the while loop
    Logger::debug("[Server socket][" + intToString(fd) + "] initialized with epoll");
}

/**
 * @brief Creates an epoll instance and adds the server socket fd to monitor for incoming connections.
 * @param fd - File descriptor of the server socket to add to epoll.
 * @throws std::runtime_error if epoll creation or adding the fd fails.
 */
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

/**
 * @brief Adds a client socket fd to the epoll instance for monitoring with edge-triggered input events.
 * @param epoll_fd - File descriptor of the epoll instance.
 * @param fd - File descriptor of the client socket to add.
 * @throws std::runtime_error if adding the client fd to epoll fails (also closes the fd).
 */
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

/**
 * @brief Accepts a new client connection, sets it non-blocking, adds it to epoll,
 *        and stores it in the client map.
 * @throws runtime_error or logs error if client creation or epoll addition fails.
 */
void ServerManager::createClient()
{
    try 
    {
        Client client(serverSocket);
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
    int fd = client.getClientFd();
    try
    {
        client.getRequest();
        ParserManager pm;
        pm.parseRequest(client);
        pm.buildResponse(serverSocket, client);
        Logger::info("[" + intToString(fd) + "] request and response completed");
    }
    catch (const std::exception& e)
    {
        Logger::error("Client [" + intToString(fd) + "] handling failed: " + e.what());
        if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1)
            perror("epoll_ctl DEL");
        close(fd);
        clients.erase(fd);
        Logger::info("Closed and removed client fd " + intToString(fd));
    }
}

/**
 * @brief Main event loop: waits for epoll events and dispatches accordingly.
 *        If event is on server socket, accepts new client.
 *        Otherwise, processes activity on client sockets.
 */
void ServerManager::run()
{
    const int MAX_EVENTS = 64;
    epoll_event events[MAX_EVENTS];
    
    Logger::info("Server started, entering main loop...");
    
    while (running)
    {
        std::cout << "checking ...\n";
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n == -1) 
        {
            if (errno == EINTR) {
		        Logger::info("epoll_wait interrupted by signal, shutting down.");
		        break; // graceful break
	        }
            Logger::error("epoll_wait() failed");
            continue;
        }
        
        Logger::debug("epoll_wait() returned " + intToString(n) + " ready fd(s)");
        for (int i = 0; i < n; ++i) 
        {
            int fd = events[i].data.fd;
            
            if (fd == serverSocket.getServerFd()) 
            {
                Logger::info("[" + intToString(fd) + "] New connection");
                createClient();
            } 
            else 
            {
                //Logger::info("[" + intToString(fd) + "] Client verification " + );
                std::map<int, Client>::iterator it = clients.find(fd);
                if (it == clients.end())
                {
                    Logger::error("Client fd " + intToString(fd) + " not found");
                    continue;
                }
                handleClient(it->second);
            }
        }
    }
}

void ServerManager::stop() {
    running = false;
}