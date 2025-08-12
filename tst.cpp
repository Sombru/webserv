// =====================================================
// MultiServerWebServer.h
// =====================================================

#ifndef MULTI_SERVER_WEBSERVER_H
#define MULTI_SERVER_WEBSERVER_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <functional>

class MultiServerWebServer
{
public:
	// Server configuration structure
	struct ServerConfig
	{
		int port;
		std::string interface; // IP address to bind to ("0.0.0.0" for all)
		std::string name;	   // Human-readable name for this server

		ServerConfig(int p, const std::string &iface = "0.0.0.0", const std::string &n = "")
			: port(p), interface(iface), name(n.empty() ? "Server-" + std::to_string(p) : n) {}
	};

	// Request handler function type
	using RequestHandler = std::function<std::string(const std::string &method,
													 const std::string &path,
													 const std::string &server_name,
													 int server_port)>;

public:
	MultiServerWebServer();
	~MultiServerWebServer();

	// Server management
	bool addServer(const ServerConfig &config);
	bool addServer(int port, const std::string &interface = "0.0.0.0", const std::string &name = "");
	void setRequestHandler(RequestHandler handler);

	// Main interface
	bool start();
	void run();
	void stop();

	// Configuration
	void setKeepAliveTimeout(int seconds);
	void setMaxEvents(int max_events);

	// Statistics
	size_t getActiveConnections() const;
	std::vector<std::string> getServerInfo() const;

private:
	// Configuration constants
	static const int DEFAULT_TIMEOUT_SECONDS = 30;
	static const int DEFAULT_MAX_EVENTS = 1000;

	// Server information structure
	struct ServerInfo
	{
		int fd;
		ServerConfig config;
		size_t connection_count;

		ServerInfo(int socket_fd, const ServerConfig &cfg)
			: fd(socket_fd), config(cfg), connection_count(0) {}
	};

	// Client information structure
	struct ClientInfo
	{
		std::chrono::steady_clock::time_point last_activity;
		std::string buffer;
		int server_fd; // Which server this client connected to

		ClientInfo(int srv_fd);
	};

	// Member variables
	int epoll_fd_;
	int timeout_seconds_;
	int max_events_;
	bool running_;

	std::vector<ServerConfig> server_configs_;
	std::unordered_map<int, ServerInfo> servers_; // fd -> ServerInfo
	std::unordered_map<int, ClientInfo> clients_; // client_fd -> ClientInfo
	std::unordered_set<int> server_fds_;		  // Quick lookup for server sockets

	RequestHandler request_handler_;

	// Socket operations
	bool createAndBindServer(const ServerConfig &config);
	bool setupEpoll();
	bool setNonBlocking(int fd);

	// Event handling
	void handleNewConnection(int server_fd);
	void handleClientData(int client_fd);
	void processHttpRequests(int client_fd, ClientInfo &client);

	// HTTP processing
	void handleHttpRequest(int client_fd, const std::string &request, const ServerInfo &server_info);
	std::string createDefaultResponseBody(const std::string &method, const std::string &path,
										  const std::string &server_name, int server_port);
	std::string createHttpResponse(const std::string &body, bool keep_alive);
	void sendResponse(int client_fd, const std::string &response);

	// Connection management
	void cleanupTimeouts();
	void removeClient(int client_fd);
	void cleanup();

	// Utility
	void setupSignalHandling();
	std::string getServerName(int server_fd) const;
	int getServerPort(int server_fd) const;
};

#endif // MULTI_SERVER_WEBSERVER_H

// =====================================================
// MultiServerWebServer.cpp
// =====================================================

#include <iostream>
#include <sstream>
#include <algorithm>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include <signal.h>

// =====================================================
// Constructor and Destructor
// =====================================================

MultiServerWebServer::MultiServerWebServer()
	: epoll_fd_(-1), timeout_seconds_(DEFAULT_TIMEOUT_SECONDS), max_events_(DEFAULT_MAX_EVENTS), running_(false)
{
	setupSignalHandling();
}

MultiServerWebServer::~MultiServerWebServer()
{
	cleanup();
}

// =====================================================
// ClientInfo Implementation
// =====================================================

MultiServerWebServer::ClientInfo::ClientInfo(int srv_fd)
	: last_activity(std::chrono::steady_clock::now()), server_fd(srv_fd)
{
}

// =====================================================
// Public Interface
// =====================================================

bool MultiServerWebServer::addServer(const ServerConfig &config)
{
	if (running_)
	{
		std::cerr << "Cannot add servers while running" << std::endl;
		return false;
	}

	server_configs_.push_back(config);
	return true;
}

bool MultiServerWebServer::addServer(int port, const std::string &interface, const std::string &name)
{
	return addServer(ServerConfig(port, interface, name));
}

void MultiServerWebServer::setRequestHandler(RequestHandler handler)
{
	request_handler_ = handler;
}

bool MultiServerWebServer::start()
{
	if (server_configs_.empty())
	{
		std::cerr << "No servers configured" << std::endl;
		return false;
	}

	if (!setupEpoll())
	{
		return false;
	}

	// Create and bind all server sockets
	for (const auto &config : server_configs_)
	{
		if (!createAndBindServer(config))
		{
			cleanup();
			return false;
		}
	}

	running_ = true;
	std::cout << "Multi-server started with " << servers_.size() << " server(s)" << std::endl;
	return true;
}

void MultiServerWebServer::run()
{
	if (!running_)
	{
		std::cerr << "Server not started" << std::endl;
		return;
	}

	std::vector<struct epoll_event> events(max_events_);

	std::cout << "Listening on:" << std::endl;
	for (const auto &pair : servers_)
	{
		const auto &info = pair.second;
		std::cout << "  " << info.config.name << " -> "
				  << info.config.interface << ":" << info.config.port << std::endl;
	}
	std::cout << "Keep-alive timeout: " << timeout_seconds_ << " seconds" << std::endl;
	std::cout << "Press Ctrl+C to stop" << std::endl;

	while (running_)
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
			int fd = events[i].data.fd;

			if (server_fds_.count(fd))
			{
				// This is a server socket - new connection
				handleNewConnection(fd);
			}
			else
			{
				// This is a client socket - data available
				handleClientData(fd);
			}
		}

		// Cleanup expired connections
		cleanupTimeouts();
	}
}

void MultiServerWebServer::stop()
{
	running_ = false;
}

void MultiServerWebServer::setKeepAliveTimeout(int seconds)
{
	timeout_seconds_ = seconds;
}

void MultiServerWebServer::setMaxEvents(int max_events)
{
	max_events_ = max_events;
}

size_t MultiServerWebServer::getActiveConnections() const
{
	return clients_.size();
}

std::vector<std::string> MultiServerWebServer::getServerInfo() const
{
	std::vector<std::string> info;
	for (const auto &pair : servers_)
	{
		const auto &server = pair.second;
		std::ostringstream oss;
		oss << server.config.name << " (" << server.config.interface << ":" << server.config.port << ") - "
			<< server.connection_count << " connections";
		info.push_back(oss.str());
	}
	return info;
}

// =====================================================
// Socket Setup
// =====================================================

bool MultiServerWebServer::createAndBindServer(const ServerConfig &config)
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
	{
		std::cerr << "Failed to create socket for " << config.name
				  << ": " << strerror(errno) << std::endl;
		return false;
	}

	// Enable socket reuse
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		std::cerr << "Failed to set socket options for " << config.name
				  << ": " << strerror(errno) << std::endl;
		close(server_fd);
		return false;
	}

	if (!setNonBlocking(server_fd))
	{
		close(server_fd);
		return false;
	}

	// Bind socket
	struct sockaddr_in address;
	std::memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(config.port);

	if (config.interface == "0.0.0.0" || config.interface.empty())
	{
		address.sin_addr.s_addr = INADDR_ANY;
	}
	else
	{
		if (inet_pton(AF_INET, config.interface.c_str(), &address.sin_addr) <= 0)
		{
			std::cerr << "Invalid interface address: " << config.interface << std::endl;
			close(server_fd);
			return false;
		}
	}

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		std::cerr << "Bind failed for " << config.name << " on "
				  << config.interface << ":" << config.port
				  << ": " << strerror(errno) << std::endl;
		close(server_fd);
		return false;
	}

	if (listen(server_fd, SOMAXCONN) < 0)
	{
		std::cerr << "Listen failed for " << config.name
				  << ": " << strerror(errno) << std::endl;
		close(server_fd);
		return false;
	}

	// Add to epoll
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = server_fd;

	if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, server_fd, &event) < 0)
	{
		std::cerr << "Failed to add " << config.name << " to epoll: "
				  << strerror(errno) << std::endl;
		close(server_fd);
		return false;
	}

	// Store server info
	servers_.emplace(server_fd, ServerInfo(server_fd, config));
	server_fds_.insert(server_fd);

	std::cout << "Server " << config.name << " bound to "
			  << config.interface << ":" << config.port << std::endl;

	return true;
}

bool MultiServerWebServer::setupEpoll()
{
	epoll_fd_ = epoll_create1(0);
	if (epoll_fd_ < 0)
	{
		std::cerr << "Failed to create epoll: " << strerror(errno) << std::endl;
		return false;
	}
	return true;
}

bool MultiServerWebServer::setNonBlocking(int fd)
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

void MultiServerWebServer::handleNewConnection(int server_fd)
{
	auto server_it = servers_.find(server_fd);
	if (server_it == servers_.end())
	{
		return;
	}

	ServerInfo &server_info = server_it->second;

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
			std::cerr << "Accept failed on " << server_info.config.name
					  << ": " << strerror(errno) << std::endl;
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

		clients_.emplace(client_fd, ClientInfo(server_fd));
		server_info.connection_count++;

		char client_ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);

		std::cout << "New client " << client_fd << " connected to "
				  << server_info.config.name << " from " << client_ip
				  << ":" << ntohs(client_addr.sin_port)
				  << " (Total: " << clients_.size() << ")" << std::endl;
	}
}

void MultiServerWebServer::handleClientData(int client_fd)
{
	auto client_it = clients_.find(client_fd);
	if (client_it == clients_.end())
	{
		return;
	}

	ClientInfo &client = client_it->second;
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

void MultiServerWebServer::processHttpRequests(int client_fd, ClientInfo &client)
{
	size_t pos = 0;

	while ((pos = client.buffer.find("\r\n\r\n")) != std::string::npos)
	{
		std::string request = client.buffer.substr(0, pos + 4);
		client.buffer.erase(0, pos + 4);

		auto server_it = servers_.find(client.server_fd);
		if (server_it != servers_.end())
		{
			handleHttpRequest(client_fd, request, server_it->second);
		}
	}
}

// =====================================================
// HTTP Processing
// =====================================================

void MultiServerWebServer::handleHttpRequest(int client_fd, const std::string &request,
											 const ServerInfo &server_info)
{
	std::cout << "Processing request from client " << client_fd
			  << " on " << server_info.config.name << std::endl;

	// Parse request line
	std::istringstream iss(request);
	std::string method, path, version;
	iss >> method >> path >> version;

	// Generate response body using custom handler or default
	std::string response_body;
	if (request_handler_)
	{
		response_body = request_handler_(method, path, server_info.config.name, server_info.config.port);
	}
	else
	{
		response_body = createDefaultResponseBody(method, path, server_info.config.name, server_info.config.port);
	}

	std::string response = createHttpResponse(response_body, true);
	sendResponse(client_fd, response);
}

std::string MultiServerWebServer::createDefaultResponseBody(const std::string &method,
															const std::string &path,
															const std::string &server_name,
															int server_port)
{
	auto now = std::chrono::system_clock::now();
	auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
						 now.time_since_epoch())
						 .count();

	std::ostringstream body;
	body << "<!DOCTYPE html>\n"
		 << "<html><head><title>Multi-Server: " << server_name << "</title></head>\n"
		 << "<body style='font-family: Arial, sans-serif; margin: 40px;'>\n"
		 << "<h1>🌐 Multi-Server Web Server</h1>\n"
		 << "<div style='background: #f0f8ff; padding: 20px; border-radius: 8px; margin: 20px 0;'>\n"
		 << "<h2>Server: " << server_name << "</h2>\n"
		 << "<p><strong>Port:</strong> " << server_port << "</p>\n"
		 << "<p><strong>Method:</strong> " << method << "</p>\n"
		 << "<p><strong>Path:</strong> " << path << "</p>\n"
		 << "<p><strong>Timestamp:</strong> " << timestamp << "</p>\n"
		 << "</div>\n"
		 << "<div style='background: #f5f5f5; padding: 15px; border-radius: 8px;'>\n"
		 << "<h3>Server Statistics</h3>\n";

	for (const auto &info : getServerInfo())
	{
		body << "<p>• " << info << "</p>\n";
	}

	body << "<p><strong>Total Active Connections:</strong> " << clients_.size() << "</p>\n"
		 << "<p><strong>Keep-Alive Timeout:</strong> " << timeout_seconds_ << " seconds</p>\n"
		 << "</div>\n"
		 << "<p><em>This connection will remain open until timeout or explicit close.</em></p>\n"
		 << "</body></html>";

	return body.str();
}

std::string MultiServerWebServer::createHttpResponse(const std::string &body, bool keep_alive)
{
	std::ostringstream response;

	response << "HTTP/1.1 200 OK\r\n"
			 << "Content-Type: text/html; charset=UTF-8\r\n"
			 << "Content-Length: " << body.length() << "\r\n"
			 << "Server: MultiServerWebServer/1.0\r\n";

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

void MultiServerWebServer::sendResponse(int client_fd, const std::string &response)
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

void MultiServerWebServer::cleanupTimeouts()
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

void MultiServerWebServer::removeClient(int client_fd)
{
	auto client_it = clients_.find(client_fd);
	if (client_it != clients_.end())
	{
		// Decrease connection count for the server
		int server_fd = client_it->second.server_fd;
		auto server_it = servers_.find(server_fd);
		if (server_it != servers_.end())
		{
			server_it->second.connection_count--;
		}
	}

	if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, client_fd, nullptr) < 0)
	{
		std::cerr << "Failed to remove client " << client_fd
				  << " from epoll: " << strerror(errno) << std::endl;
	}

	close(client_fd);
	clients_.erase(client_fd);
}

void MultiServerWebServer::cleanup()
{
	running_ = false;

	// Close all client connections
	for (const auto &pair : clients_)
	{
		close(pair.first);
	}
	clients_.clear();

	// Close all server sockets
	for (const auto &pair : servers_)
	{
		close(pair.first);
	}
	servers_.clear();
	server_fds_.clear();

	if (epoll_fd_ >= 0)
	{
		close(epoll_fd_);
		epoll_fd_ = -1;
	}
}

void MultiServerWebServer::setupSignalHandling()
{
	signal(SIGPIPE, SIG_IGN);
}

std::string MultiServerWebServer::getServerName(int server_fd) const
{
	auto it = servers_.find(server_fd);
	return (it != servers_.end()) ? it->second.config.name : "Unknown";
}

int MultiServerWebServer::getServerPort(int server_fd) const
{
	auto it = servers_.find(server_fd);
	return (it != servers_.end()) ? it->second.config.port : -1;
}

// =====================================================
// main.cpp - Example Usage
// =====================================================

int main(int argc, char *argv[])
{
	try
	{
		MultiServerWebServer server;

		// Add multiple servers
		server.addServer(8080, "0.0.0.0", "Main-HTTP");
		server.addServer(8443, "0.0.0.0", "Secure-HTTPS");
		server.addServer(9000, "127.0.0.1", "Admin-Local");

		// Optional: Set custom request handler
		server.setRequestHandler([](const std::string &method, const std::string &path,
									const std::string &server_name, int server_port) -> std::string
								 {
            std::ostringstream response;
            
            if (server_name == "Admin-Local" && path == "/admin") {
                response << "<h1>Admin Panel</h1><p>Welcome to the admin interface!</p>";
            } else if (server_port == 8443) {
                response << "<h1>Secure Server</h1><p>This would be HTTPS in production</p>";
            } else if (path == "/api/status") {
                response << "{\"status\": \"ok\", \"server\": \"" << server_name << "\"}";
            } else {
                // Return empty string to use default handler
                return "";
            }
            
            return response.str(); });

		// Optional: Configure settings
		server.setKeepAliveTimeout(45);
		server.setMaxEvents(2000);

		if (!server.start())
		{
			std::cerr << "Failed to start multi-server" << std::endl;
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