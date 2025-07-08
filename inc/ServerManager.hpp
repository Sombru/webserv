#pragma once

#include "Webserv.hpp"
#include "Config.hpp"
#include "Socket.hpp"

class Socket;
class Client;

class ServerManager
{
private:
    std::vector<ServerConfig> servers; // passing a vector here to create a new Server Manager per server block
	std::map<int, ServerConfig> serverSockets;  // fd -> ServerConfig
    std::map<int, Client> clients;              // fd -> Client
    //struct ServerConfig& server;
	int epoll_fd;
	//std::map<int, Client> clients;
    bool running;
	//Socket socket;

	void set_non_blocking(int fd);
	void setupEpoll(int fd);
    void addFdToEpoll(int fd);
	void handleClient(int fd);
    void acceptNewClient(int server_fd);
    //void handleClient(Client& client);
	//void addClientToEpoll(int epoll_fd, int fd);
	
public:

	//bool running;

	//ServerManager(ServerConfig& serv);
    ServerManager(const std::vector<ServerConfig>& serverConfigs);
	~ServerManager();

	void serverLoop();


	//void createClient();

	//ServerConfig& getServer() const;
};

