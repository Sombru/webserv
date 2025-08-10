// =====================================================
// KeepAliveWebServer.h
// =====================================================

#ifndef KEEPALIVE_WEBSERVER_H
#define KEEPALIVE_WEBSERVER_H

#include <string>
#include <unordered_map>
#include <chrono>

class KeepAliveWebServer
{
public:
	explicit KeepAliveWebServer(int port);
	~KeepAliveWebServer();

	// Main interface
	bool start();
	void run();

	// Configuration
	void setKeepAliveTimeout(int seconds);
	void setMaxEvents(int max_events);

private:
	// Configuration constants
	static const int DEFAULT_TIMEOUT_SECONDS = 30;
	static const int DEFAULT_MAX_EVENTS = 1000;

	// Member variables
	int server_fd_;
	int epoll_fd_;
	int port_;
	int timeout_seconds_;
	int max_events_;

	// Client information structure
	struct ClientInfo
	{
		std::chrono::steady_clock::time_point last_activity;
		std::string buffer;

		ClientInfo();
	};

	std::unordered_map<int, ClientInfo> clients_;

	// Socket operations
	bool createServerSocket();
	bool bindAndListen();
	bool setupEpoll();
	bool setNonBlocking(int fd);

	// Event handling
	void handleNewConnection();
	void handleClientData(int client_fd);
	void processHttpRequests(int client_fd, ClientInfo &client);

	// HTTP processing
	void handleHttpRequest(int client_fd, const std::string &request);
	std::string createResponseBody(const std::string &method, const std::string &path);
	std::string createHttpResponse(const std::string &body, bool keep_alive);
	void sendResponse(int client_fd, const std::string &response);

	// Connection management
	void cleanupTimeouts();
	void removeClient(int client_fd);
	void cleanup();

	// Utility
	void setupSignalHandling();
};

#endif // KEEPALIVE_WEBSERVER_H

// =====================================================
// KeepAliveWebServer.cpp
// =====================================================

// #include "KeepAliveWebServer.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include <signal.h>

// =====================================================
// Constructor and Destructor
// =====================================================

KeepAliveWebServer::KeepAliveWebServer(int port)
	: server_fd_(-1), epoll_fd_(-1), port_(port), timeout_seconds_(DEFAULT_TIMEOUT_SECONDS), max_events_(DEFAULT_MAX_EVENTS)
{
	setupSignalHandling();
}

KeepAliveWebServer::~KeepAliveWebServer()
{
	cleanup();
}

// =====================================================
// ClientInfo Implementation
// =====================================================

KeepAliveWebServer::ClientInfo::ClientInfo()
	: last_activity(std::chrono::steady_clock::now())
{
}

// =====================================================
// Public Interface
// =====================================================

bool KeepAliveWebServer::start()
{
	return createServerSocket() &&
		   bindAndListen() &&
		   setupEpoll();
}

void KeepAliveWebServer::run()
{
	std::vector<struct epoll_event> events(max_events_);

	std::cout << "Server running on port " << port_ << std::endl;
	std::cout << "Keep-alive timeout: " << timeout_seconds_ << " seconds" << std::endl;
	std::cout << "Press Ctrl+C to stop" << std::endl;

	while (true)
	{
		int num_events = epoll_wait(epoll_fd_, events.data(), max_events_, 1000);

		if (num_events < 0)
		{
			if (errno == EINTR)
				continue;
			std::cerr << "epoll_wait failed: " << strerror(errno) << std::endl;
			break;
		}

		// Handle all events
		for (int i = 0; i < num_events; i++)
		{
			if (events[i].data.fd == server_fd_)
			{
				handleNewConnection();
			}
			else
			{
				handleClientData(events[i].data.fd);
			}
		}

		// Cleanup expired connections
		cleanupTimeouts();
	}
}

void KeepAliveWebServer::setKeepAliveTimeout(int seconds)
{
	timeout_seconds_ = seconds;
}

void KeepAliveWebServer::setMaxEvents(int max_events)
{
	max_events_ = max_events;
}

// =====================================================
// Socket Setup
// =====================================================

bool KeepAliveWebServer::createServerSocket()
{
	server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd_ < 0)
	{
		std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
		return false;
	}

	// Enable socket reuse
	int opt = 1;
	if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		std::cerr << "Failed to set socket options: " << strerror(errno) << std::endl;
		return false;
	}

	return setNonBlocking(server_fd_);
}

bool KeepAliveWebServer::bindAndListen()
{
	struct sockaddr_in address;
	std::memset(&address, 0, sizeof(address));

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port_);

	if (bind(server_fd_, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		std::cerr << "Bind failed: " << strerror(errno) << std::endl;
		return false;
	}

	if (listen(server_fd_, SOMAXCONN) < 0)
	{
		std::cerr << "Listen failed: " << strerror(errno) << std::endl;
		return false;
	}

	return true;
}

bool KeepAliveWebServer::setupEpoll()
{
	epoll_fd_ = epoll_create1(0);
	if (epoll_fd_ < 0)
	{
		std::cerr << "Failed to create epoll: " << strerror(errno) << std::endl;
		return false;
	}

	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = server_fd_;

	if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, server_fd_, &event) < 0)
	{
		std::cerr << "Failed to add server socket to epoll: " << strerror(errno) << std::endl;
		return false;
	}

	return true;
}

bool KeepAliveWebServer::setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
	{
		std::cerr << "Failed to get socket flags: " << strerror(errno) << std::endl;
		return false;
	}

	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		std::cerr << "Failed to set non-blocking: " << strerror(errno) << std::endl;
		return false;
	}

	return true;
}

// =====================================================
// Event Handling
// =====================================================

void KeepAliveWebServer::handleNewConnection()
{
	while (true)
	{
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);

		int client_fd = accept(server_fd_, (struct sockaddr *)&client_addr, &client_len);
		if (client_fd < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				break; // No more connections
			}
			std::cerr << "Accept failed: " << strerror(errno) << std::endl;
			break;
		}

		if (!setNonBlocking(client_fd))
		{
			close(client_fd);
			continue;
		}

		// Add client to epoll
		struct epoll_event event;
		event.events = EPOLLIN | EPOLLET; // Edge-triggered
		event.data.fd = client_fd;

		if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &event) < 0)
		{
			std::cerr << "Failed to add client to epoll: " << strerror(errno) << std::endl;
			close(client_fd);
			continue;
		}

		clients_[client_fd] = ClientInfo();
		std::cout << "New client connected: " << client_fd << " (Total clients: " << clients_.size() << ")" << std::endl;
	}
}

void KeepAliveWebServer::handleClientData(int client_fd)
{
	auto it = clients_.find(client_fd);
	if (it == clients_.end())
	{
		return;
	}

	ClientInfo &client = it->second;
	client.last_activity = std::chrono::steady_clock::now();

	char buffer[4096];

	while (true)
	{
		ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);

		if (bytes_read < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				break; // No more data
			}
			std::cerr << "Recv failed for client " << client_fd
					  << ": " << strerror(errno) << std::endl;
			removeClient(client_fd);
			return;
		}

		if (bytes_read == 0)
		{
			std::cout << "Client " << client_fd << " disconnected gracefully" << std::endl;
			removeClient(client_fd);
			return;
		}

		client.buffer.append(buffer, bytes_read);
		processHttpRequests(client_fd, client);
	}
}

void KeepAliveWebServer::processHttpRequests(int client_fd, ClientInfo &client)
{
	size_t pos = 0;

	while ((pos = client.buffer.find("\r\n\r\n")) != std::string::npos)
	{
		std::string request = client.buffer.substr(0, pos + 4);
		client.buffer.erase(0, pos + 4);

		handleHttpRequest(client_fd, request);
	}
}

// =====================================================
// HTTP Processing
// =====================================================

void KeepAliveWebServer::handleHttpRequest(int client_fd, const std::string &request)
{
	std::cout << "Processing request from client " << client_fd << std::endl;

	// Parse request line
	std::istringstream iss(request);
	std::string method, path, version;
	iss >> method >> path >> version;

	// Create and send response
	std::string response_body = createResponseBody(method, path);
	std::string response = createHttpResponse(response_body, true);
	sendResponse(client_fd, response);
}

std::string KeepAliveWebServer::createResponseBody(const std::string &method, const std::string &path)
{
	auto now = std::chrono::system_clock::now();
	auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
						 now.time_since_epoch())
						 .count();

	std::ostringstream body;
	body << "<!DOCTYPE html>\n"
		 << "<html><head><title>Keep-Alive Server</title></head>\n"
		 << "<body style='font-family: Arial, sans-serif; margin: 40px;'>\n"
		 << "<h1>🚀 Keep-Alive Web Server</h1>\n"
		 << "<div style='background: #f0f0f0; padding: 20px; border-radius: 8px;'>\n"
		 << "<p><strong>Method:</strong> " << method << "</p>\n"
		 << "<p><strong>Path:</strong> " << path << "</p>\n"
		 << "<p><strong>Timestamp:</strong> " << timestamp << "</p>\n"
		 << "<p><strong>Active Connections:</strong> " << clients_.size() << "</p>\n"
		 << "<p><strong>Keep-Alive Timeout:</strong> " << timeout_seconds_ << " seconds</p>\n"
		 << "</div>\n"
		 << "<p><em>This connection will remain open until timeout or explicit close.</em></p>\n"
		 << "</body></html>";

	return body.str();
}

std::string KeepAliveWebServer::createHttpResponse(const std::string &body, bool keep_alive)
{
	std::ostringstream response;

	response << "HTTP/1.1 200 OK\r\n"
			 << "Content-Type: text/html; charset=UTF-8\r\n"
			 << "Content-Length: " << body.length() << "\r\n"
			 << "Server: KeepAliveWebServer/1.0\r\n";

	if (keep_alive)
	{
		response << "Connection: keep-alive\r\n"
				 << "Keep-Alive: timeout=" << timeout_seconds_ << ", max=1000\r\n";
	}
	else
	{
		response << "Connection: close\r\n";
	}

	response << "\r\n"
			 << body;
	return response.str();
}

void KeepAliveWebServer::sendResponse(int client_fd, const std::string &response)
{
	const char *data = response.c_str();
	size_t total_sent = 0;
	size_t total_size = response.length();

	while (total_sent < total_size)
	{
		ssize_t sent = send(client_fd, data + total_sent, total_size - total_sent, MSG_NOSIGNAL);

		if (sent < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				usleep(1000); // Wait 1ms and retry
				continue;
			}
			std::cerr << "Send failed for client " << client_fd
					  << ": " << strerror(errno) << std::endl;
			removeClient(client_fd);
			return;
		}

		total_sent += sent;
	}
}

// =====================================================
// Connection Management
// =====================================================

void KeepAliveWebServer::cleanupTimeouts()
{
	auto now = std::chrono::steady_clock::now();
	std::vector<int> to_remove;

	for (const auto &pair : clients_)
	{
		int client_fd = pair.first;
		const ClientInfo &client = pair.second;

		auto duration = std::chrono::duration_cast<std::chrono::seconds>(
							now - client.last_activity)
							.count();

		if (duration >= timeout_seconds_)
		{
			std::cout << "Client " << client_fd << " timed out after "
					  << duration << " seconds" << std::endl;
			to_remove.push_back(client_fd);
		}
	}

	for (int client_fd : to_remove)
	{
		removeClient(client_fd);
	}
}

void KeepAliveWebServer::removeClient(int client_fd)
{
	if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, client_fd, nullptr) < 0)
	{
		std::cerr << "Failed to remove client " << client_fd
				  << " from epoll: " << strerror(errno) << std::endl;
	}

	close(client_fd);
	clients_.erase(client_fd);
}

void KeepAliveWebServer::cleanup()
{
	// Close all client connections
	for (const auto &pair : clients_)
	{
		close(pair.first);
	}
	clients_.clear();

	if (epoll_fd_ >= 0)
	{
		close(epoll_fd_);
		epoll_fd_ = -1;
	}

	if (server_fd_ >= 0)
	{
		close(server_fd_);
		server_fd_ = -1;
	}
}

void KeepAliveWebServer::setupSignalHandling()
{
	// Ignore SIGPIPE to handle broken connections gracefully
	signal(SIGPIPE, SIG_IGN);
}

// =====================================================
// main.cpp
// =====================================================

int main(int argc, char *argv[])
{
	int port = 8080;

	if (argc > 1)
	{
		port = std::atoi(argv[1]);
		if (port <= 0 || port > 65535)
		{
			std::cerr << "Error: Invalid port number. Use 1-65535." << std::endl;
			return 1;
		}
	}

	try
	{
		KeepAliveWebServer server(port);

		// Optional: Configure server settings
		// server.setKeepAliveTimeout(60);  // 60 seconds
		// server.setMaxEvents(2000);       // Handle more events

		if (!server.start())
		{
			std::cerr << "Failed to start server" << std::endl;
			return 1;
		}

		server.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Server error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}