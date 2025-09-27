#include "Server.hpp"
#include "ServerManager.hpp"
#include "Client.hpp"
#include "HTTP.hpp"
#include "Logger.hpp"

Server::Server()
: port(0), server_fd(-1)
{
}

Server::Server(ServerConfig &serverSrc)
: serverConfig(serverSrc), server_fd(-1)
{
	addresStr = (serverConfig.host.substr(0, serverConfig.host.find(':')));
	port = atoi(serverConfig.host.substr(serverConfig.host.find(':') + 1).c_str());
}

Server::Server(const Server& other)
: port(other.port), addresStr(other.addresStr), serverConfig(other.serverConfig), server_fd(other.server_fd)
{
}

Server& Server::operator=(const Server& other)
{
	if (this != &other)
	{
		port = other.port;
		addresStr = other.addresStr;
		serverConfig = other.serverConfig;
		server_fd = other.server_fd;
	}
	return *this;
}

Server::~Server() {}

bool Server::setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0); // GETFLAGS (GETFLAGS NOT ALLOWED)
	if (flags < 0)
	{
		ERROR("Failed to get Server flags: " + errstr);
		return false;
	}

	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) // SETFLAGS of this fd to nonblocking
	{
		ERROR("Failed to set non-blocking: " + errstr);
		return false;
	}
	return true;
}

int Server::setup()
{
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
	{
		ERROR("Could not create Server: " + errstr);
		return -1;
	}
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		ERROR("Failed to set Server options for " + serverConfig.name + ": " + errstr);
		close(server_fd);
		return false;
	}

	if (!setNonBlocking(server_fd))
		return -1;
	struct addrinfo hints;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo *res;
	if (getaddrinfo(addresStr.c_str(), intToString(port).c_str(), &hints, &res) < 0)
	{
		ERROR("Failed to get addres info for " + serverConfig.name + ": " + errstr);
		return -1;
	}

	if (bind(server_fd, res->ai_addr, res->ai_addrlen) < 0)
	{
		freeaddrinfo(res);
		ERROR("Failed to bind Server for " + serverConfig.name + ": " + errstr);
		return -1;
	}
	freeaddrinfo(res); // free result after use

	if (listen(server_fd, MAX_CONNECTIONS) < 0)
	{
		ERROR("Failed to listen for connection for " + serverConfig.name + ": " + errstr);
		return -1;
	}
	return EXIT_SUCCESS;
}

void Server::acceptConnection(int &epoll_fd, std::map<int, Client> &clientsMap) 
{
	while (true)
	{
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);

		int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
		if (client_fd < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				break; // No more connections
			}
			ERROR("Accept failed: " + errstr);
			break;
		}

		if (!setNonBlocking(client_fd))
			return ;

		struct epoll_event event;
		event.events = EPOLLIN | EPOLLET;
		event.data.fd = client_fd;
		
		if ((epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) < 0))
		{
			ERROR("Failed to add clinet to epoll: " + errstr);
			close(client_fd);
			return;
		}
		clientsMap[client_fd] = this;

		char clinetIP[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &client_addr.sin_addr, clinetIP, INET_ADDRSTRLEN);
		INFO("New clinet " + intToString(client_fd) + " connected to " + serverConfig.name + " at " + getTimestamp());
		INFO((std::string)"Client IP: " + clinetIP);
	}
}

bool Server::handleConnection(int fd)
{
	char buffer[4096];
	std::string rawRequest;

	while (true)
	{
		ssize_t bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0);
		if (bytesRead < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				break; // No more data available right now
			}
			ERROR("Error reading from client " + intToString(fd) + ": " + errstr);
			return false; // Signal to remove client	
		}

		if (bytesRead == 0)
		{
			// INFO("Client " + intToString(fd) + " disconnected");
			return false; // Signal to remove client
		}

		// Null-terminate the buffer for safety
		buffer[bytesRead] = '\0';
		rawRequest += buffer;
		
		// Check if we have a complete HTTP request (ends with \r\n\r\n)
		if (rawRequest.find("\r\n\r\n") != std::string::npos || 
			rawRequest.find("\n\n") != std::string::npos)
		{
			// We have a complete request, process it
			HTTP httpHandler(rawRequest, serverConfig);
			httpHandler.parseRequest();
			
			const HttpRequest& request = httpHandler.request;
			
			// Log the parsed request
			INFO("HTTP Request - Method: " + request.method + 
				 ", Path: " + request.path + 
				 ", Version: " + request.version);
			// Generate configured response (serves index/error/success per config)
			httpHandler.generateResponse();
			const HttpResponse& resp = httpHandler.response;
			
			// Build raw HTTP response
			std::string response = resp.version + " " + intToString(resp.status_code) + " " + resp.status_text + "\r\n";
			for (std::map<std::string, std::string>::const_iterator it = httpHandler.response.headers.begin(); it != httpHandler.response.headers.end(); ++it)
			{
				response += it->first + ": " + it->second + "\r\n";
			}
			response += "\r\n";
			response += resp.body;
			// DEBUG(response);
			
			ssize_t bytesSent = send(fd, response.c_str(), response.size(), 0);
			if (bytesSent < 0)
			{
				ERROR("Failed to send response to client " + intToString(fd) + ": " + errstr);
			}
			else
			{
				DEBUG("Sent " + intToString(bytesSent) + " bytes to client " + intToString(fd));
			}
			
			// return false; // Close connection after sending response
		}
	}
	
	return true; // Keep connection alive, waiting for more data
}
