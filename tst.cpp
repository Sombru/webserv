// =====================================================
// PerServerConfigWebServer.h
// =====================================================

#ifndef PER_SERVER_CONFIG_WEBSERVER_H
#define PER_SERVER_CONFIG_WEBSERVER_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <functional>
#include <queue>

struct ServerConfig;

class PerServerConfigWebServer
{
public:
	// Enhanced server configuration with per-server settings
	struct ServerConfig
	{
		int port;
		std::string interface;
		std::string name;
		int keep_alive_timeout; // Per-server timeout in seconds
		int max_connections;	// Per-server connection limit
		bool enable_logging;	// Per-server logging

		ServerConfig(int p, const std::string &iface = "0.0.0.0",
					 const std::string &n = "", int timeout = 30,
					 int max_conn = 1000, bool logging = true)
			: port(p), interface(iface),
			  name(n.empty() ? "Server-" + std::to_string(p) : n),
			  keep_alive_timeout(timeout), max_connections(max_conn),
			  enable_logging(logging) {}
	};

	// Request handler function type
	using RequestHandler = std::function<std::string(const std::string &method,
													 const std::string &path,
													 const ServerConfig &server_config)>;

public:
	PerServerConfigWebServer();
	~PerServerConfigWebServer();

	// Server management
	bool addServer(const ServerConfig &config);
	bool addServer(int port, const std::string &interface = "0.0.0.0",
				   const std::string &name = "", int timeout = 30,
				   int max_conn = 1000, bool logging = true);
	void setRequestHandler(RequestHandler handler);

	// Main interface
	bool start();
	void run();
	void stop();

	// Global configuration (affects all servers)
	void setGlobalMaxEvents(int max_events);
	void setCleanupInterval(int seconds);

	// Statistics
	size_t getActiveConnections() const;
	size_t getServerConnections(int server_fd) const;
	std::vector<std::string> getDetailedServerInfo() const;

private:
	// Server information with per-server tracking
	struct ServerInfo
	{
		int fd;
		ServerConfig config;
		size_t active_connections;
		std::chrono::steady_clock::time_point last_cleanup;

		// Per-server statistics
		size_t total_requests;
		size_t rejected_connections;

		ServerInfo(int socket_fd, const ServerConfig &cfg)
			: fd(socket_fd), config(cfg), active_connections(0),
			  last_cleanup(std::chrono::steady_clock::now()),
			  total_requests(0), rejected_connections(0) {}
	};

	// Enhanced client info with server-specific tracking
	struct ClientInfo
	{
		std::chrono::steady_clock::time_point last_activity;
		std::string buffer;
		int server_fd;
		int timeout_seconds; // Inherited from server config

		ClientInfo(int srv_fd, int timeout);
		bool isTimedOut() const;
	};

	// Member variables
	int epoll_fd_;
	int global_max_events_;
	int cleanup_interval_;
	bool running_;

	std::unordered_map<int, ServerInfo> servers_;
	std::unordered_map<int, ClientInfo> clients_;
	std::unordered_set<int> server_fds_;

	RequestHandler request_handler_;

	// Socket operations
	bool createAndBindServer(const ServerConfig &config);
	bool setupEpoll();
	bool setNonBlocking(int fd);

	// Event handling with per-server logic
	void handleNewConnection(int server_fd);
	void handleClientData(int client_fd);
	void processHttpRequests(int client_fd, ClientInfo &client);

	// HTTP processing
	void handleHttpRequest(int client_fd, const std::string &request,
						   const ServerInfo &server_info);
	std::string createDefaultResponseBody(const std::string &method,
										  const std::string &path,
										  const ServerConfig &config);
	std::string createHttpResponse(const std::string &body, bool keep_alive,
								   int timeout);
	void sendResponse(int client_fd, const std::string &response);

	// Per-server connection management
	void cleanupAllServers();
	void cleanupServerConnections(ServerInfo &server);
	void removeClient(int client_fd);
	void cleanup();

	// Utility
	void setupSignalHandling();
	void logMessage(const ServerConfig &config, const std::string &message);
};

#endif // PER_SERVER_CONFIG_WEBSERVER_H

// =====================================================
// PerServerConfigWebServer.cpp
// =====================================================

#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>
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

PerServerConfigWebServer::PerServerConfigWebServer()
	: epoll_fd_(-1), global_max_events_(1000), cleanup_interval_(5) // Check timeouts every 5 seconds
	  ,
	  running_(false)
{
	setupSignalHandling();
}

PerServerConfigWebServer::~PerServerConfigWebServer()
{
	cleanup();
}

// =====================================================
// ClientInfo Implementation with Per-Server Timeout
// =====================================================

PerServerConfigWebServer::ClientInfo::ClientInfo(int srv_fd, int timeout)
	: last_activity(std::chrono::steady_clock::now()), server_fd(srv_fd), timeout_seconds(timeout)
{
}

bool PerServerConfigWebServer::ClientInfo::isTimedOut() const
{
	auto now = std::chrono::steady_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::seconds>(
						now - last_activity)
						.count();
	return duration >= timeout_seconds;
}

// =====================================================
// Public Interface
// =====================================================

bool PerServerConfigWebServer::addServer(const ServerConfig &config)
{
	if (running_)
	{
		std::cerr << "Cannot add servers while running" << std::endl;
		return false;
	}

	// Validate configuration
	if (config.port <= 0 || config.port > 65535)
	{
		std::cerr << "Invalid port: " << config.port << std::endl;
		return false;
	}

	if (config.keep_alive_timeout <= 0)
	{
		std::cerr << "Invalid timeout for " << config.name
				  << ": " << config.keep_alive_timeout << std::endl;
		return false;
	}

	if (config.max_connections <= 0)
	{
		std::cerr << "Invalid max connections for " << config.name
				  << ": " << config.max_connections << std::endl;
		return false;
	}

	return createAndBindServer(config);
}

bool PerServerConfigWebServer::addServer(int port, const std::string &interface,
										 const std::string &name, int timeout,
										 int max_conn, bool logging)
{
	return addServer(ServerConfig(port, interface, name, timeout, max_conn, logging));
}

void PerServerConfigWebServer::setRequestHandler(RequestHandler handler)
{
	request_handler_ = handler;
}

bool PerServerConfigWebServer::start()
{
	if (servers_.empty())
	{
		std::cerr << "No servers configured" << std::endl;
		return false;
	}

	if (!setupEpoll())
	{
		return false;
	}

	running_ = true;
	std::cout << "Multi-server started with " << servers_.size()
			  << " server(s), using single epoll instance" << std::endl;
	return true;
}

void PerServerConfigWebServer::run()
{
	if (!running_)
	{
		std::cerr << "Server not started" << std::endl;
		return;
	}

	std::vector<struct epoll_event> events(global_max_events_);
	auto last_cleanup = std::chrono::steady_clock::now();

	std::cout << "=== SERVER CONFIGURATIONS ===" << std::endl;
	for (const auto &pair : servers_)
	{
		const auto &server = pair.second;
		std::cout << server.config.name << ":" << std::endl;
		std::cout << "  Port: " << server.config.port << std::endl;
		std::cout << "  Interface: " << server.config.interface << std::endl;
		std::cout << "  Timeout: " << server.config.keep_alive_timeout << "s" << std::endl;
		std::cout << "  Max Connections: " << server.config.max_connections << std::endl;
		std::cout << "  Logging: " << (server.config.enable_logging ? "Yes" : "No") << std::endl;
		std::cout << std::endl;
	}

	std::cout << "Global max events per epoll_wait: " << global_max_events_ << std::endl;
	std::cout << "Cleanup interval: " << cleanup_interval_ << " seconds" << std::endl;
	std::cout << "Press Ctrl+C to stop" << std::endl;

	while (running_)
	{
		// Use the global max events for epoll_wait
		int num_events = epoll_wait(epoll_fd_, events.data(), global_max_events_, 1000);

		if (num_events < 0)
		{
			if (errno == EINTR)
				continue;
			std::cerr << "epoll_wait failed: " << strerror(errno) << std::endl;
			break;
		}

		// Handle all events (could be from different servers)
		for (int i = 0; i < num_events; i++)
		{
			int fd = events[i].data.fd;

			if (server_fds_.count(fd))
			{
				handleNewConnection(fd);
			}
			else
			{
				handleClientData(fd);
			}
		}

		// Cleanup based on interval
		auto now = std::chrono::steady_clock::now();
		auto cleanup_duration = std::chrono::duration_cast<std::chrono::seconds>(
									now - last_cleanup)
									.count();

		if (cleanup_duration >= cleanup_interval_)
		{
			cleanupAllServers();
			last_cleanup = now;
		}
	}
}

void PerServerConfigWebServer::setGlobalMaxEvents(int max_events)
{
	if (!running_)
	{
		global_max_events_ = max_events;
	}
}

void PerServerConfigWebServer::setCleanupInterval(int seconds)
{
	cleanup_interval_ = seconds;
}

// =====================================================
// Enhanced Connection Handling
// =====================================================

void PerServerConfigWebServer::handleNewConnection(int server_fd)
{
	auto server_it = servers_.find(server_fd);
	if (server_it == servers_.end())
	{
		return;
	}

	ServerInfo &server_info = server_it->second;

	// Check connection limit for this specific server
	if (server_info.active_connections >= static_cast<size_t>(server_info.config.max_connections))
	{
		// Accept and immediately close to avoid connection queue buildup
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

		if (client_fd >= 0)
		{
			close(client_fd);
			server_info.rejected_connections++;

			if (server_info.config.enable_logging)
			{
				logMessage(server_info.config,
						   "Connection rejected - max connections reached (" +
							   std::to_string(server_info.config.max_connections) + ")");
			}
		}
		return;
	}

	while (true)
	{
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);

		int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
		if (client_fd < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				break;
			}
			std::cerr << "Accept failed on " << server_info.config.name
					  << ": " << strerror(errno) << std::endl;
			break;
		}

		// Check again after accepting (race condition protection)
		if (server_info.active_connections >= static_cast<size_t>(server_info.config.max_connections))
		{
			close(client_fd);
			server_info.rejected_connections++;
			continue;
		}

		if (!setNonBlocking(client_fd))
		{
			close(client_fd);
			continue;
		}

		struct epoll_event event;
		event.events = EPOLLIN | EPOLLET;
		event.data.fd = client_fd;

		if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &event) < 0)
		{
			std::cerr << "Failed to add client to epoll: " << strerror(errno) << std::endl;
			close(client_fd);
			continue;
		}

		// Create client with server-specific timeout
		clients_.emplace(client_fd, ClientInfo(server_fd, server_info.config.keep_alive_timeout));
		server_info.active_connections++;

		char client_ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);

		if (server_info.config.enable_logging)
		{
			logMessage(server_info.config,
					   "Client " + std::to_string(client_fd) + " connected from " +
						   std::string(client_ip) + ":" + std::to_string(ntohs(client_addr.sin_port)) +
						   " (timeout: " + std::to_string(server_info.config.keep_alive_timeout) + "s)");
		}
	}
}

void PerServerConfigWebServer::handleHttpRequest(int client_fd, const std::string &request,
												 const ServerInfo &server_info)
{
	std::istringstream iss(request);
	std::string method, path, version;
	iss >> method >> path >> version;

	if (server_info.config.enable_logging)
	{
		logMessage(server_info.config,
				   "Processing " + method + " " + path + " from client " + std::to_string(client_fd));
	}

	// Update server statistics
	const_cast<ServerInfo &>(server_info).total_requests++;

	std::string response_body;
	if (request_handler_)
	{
		response_body = request_handler_(method, path, server_info.config);
	}
	else
	{
		response_body = createDefaultResponseBody(method, path, server_info.config);
	}

	std::string response = createHttpResponse(response_body, true,
											  server_info.config.keep_alive_timeout);
	sendResponse(client_fd, response);
}

std::string PerServerConfigWebServer::createDefaultResponseBody(const std::string &method,
																const std::string &path,
																const ServerConfig &config)
{
	auto now = std::chrono::system_clock::now();
	auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
						 now.time_since_epoch())
						 .count();

	std::ostringstream body;
	body << "<!DOCTYPE html>\n"
		 << "<html><head><title>" << config.name << "</title></head>\n"
		 << "<body style='font-family: Arial, sans-serif; margin: 40px;'>\n"
		 << "<h1>🔧 " << config.name << " (Per-Server Config)</h1>\n"
		 << "<div style='background: #e8f5e8; padding: 20px; border-radius: 8px; margin: 20px 0;'>\n"
		 << "<h2>Request Information</h2>\n"
		 << "<p><strong>Method:</strong> " << method << "</p>\n"
		 << "<p><strong>Path:</strong> " << path << "</p>\n"
		 << "<p><strong>Timestamp:</strong> " << timestamp << "</p>\n"
		 << "</div>\n"
		 << "<div style='background: #f0f8ff; padding: 20px; border-radius: 8px; margin: 20px 0;'>\n"
		 << "<h2>Server Configuration</h2>\n"
		 << "<p><strong>Server Name:</strong> " << config.name << "</p>\n"
		 << "<p><strong>Port:</strong> " << config.port << "</p>\n"
		 << "<p><strong>Interface:</strong> " << config.interface << "</p>\n"
		 << "<p><strong>Keep-Alive Timeout:</strong> " << config.keep_alive_timeout << " seconds</p>\n"
		 << "<p><strong>Max Connections:</strong> " << config.max_connections << "</p>\n"
		 << "<p><strong>Logging Enabled:</strong> " << (config.enable_logging ? "Yes" : "No") << "</p>\n"
		 << "</div>\n";

	// Add statistics
	auto server_it = std::find_if(servers_.begin(), servers_.end(),
								  [&config](const auto &pair)
								  { return pair.second.config.port == config.port; });

	if (server_it != servers_.end())
	{
		const auto &server = server_it->second;
		body << "<div style='background: #fff3e0; padding: 20px; border-radius: 8px; margin: 20px 0;'>\n"
			 << "<h2>Server Statistics</h2>\n"
			 << "<p><strong>Active Connections:</strong> " << server.active_connections << "</p>\n"
			 << "<p><strong>Total Requests:</strong> " << server.total_requests << "</p>\n"
			 << "<p><strong>Rejected Connections:</strong> " << server.rejected_connections << "</p>\n"
			 << "</div>\n";
	}

	body << "<p><em>This connection uses server-specific timeout of "
		 << config.keep_alive_timeout << " seconds</em></p>\n"
		 << "</body></html>";

	return body.str();
}

std::string PerServerConfigWebServer::createHttpResponse(const std::string &body,
														 bool keep_alive, int timeout)
{
	std::ostringstream response;

	response << "HTTP/1.1 200 OK\r\n"
			 << "Content-Type: text/html; charset=UTF-8\r\n"
			 << "Content-Length: " << body.length() << "\r\n"
			 << "Server: PerServerConfigWebServer/1.0\r\n";

	if (keep_alive)
	{
		response << "Connection: keep-alive\r\n"
				 << "Keep-Alive: timeout=" << timeout << ", max=1000\r\n";
	}
	else
	{
		response << "Connection: close\r\n";
	}

	response << "\r\n"
			 << body;
	return response.str();
}

// =====================================================
// Per-Server Cleanup Management
// =====================================================

void PerServerConfigWebServer::cleanupAllServers()
{
	for (auto &pair : servers_)
	{
		cleanupServerConnections(pair.second);
	}
}

void PerServerConfigWebServer::cleanupServerConnections(ServerInfo &server)
{
	std::vector<int> to_remove;

	// Find clients belonging to this server that are timed out
	for (const auto &client_pair : clients_)
	{
		const auto &client = client_pair.second;

		if (client.server_fd == server.fd && client.isTimedOut())
		{
			to_remove.push_back(client_pair.first);
		}
	}

	// Remove timed-out clients
	for (int client_fd : to_remove)
	{
		if (server.config.enable_logging)
		{
			auto client_it = clients_.find(client_fd);
			if (client_it != clients_.end())
			{
				logMessage(server.config,
						   "Client " + std::to_string(client_fd) + " timed out after " +
							   std::to_string(client_it->second.timeout_seconds) + " seconds");
			}
		}
		removeClient(client_fd);
	}

	server.last_cleanup = std::chrono::steady_clock::now();
}

void PerServerConfigWebServer::removeClient(int client_fd)
{
	auto client_it = clients_.find(client_fd);
	if (client_it != clients_.end())
	{
		int server_fd = client_it->second.server_fd;
		auto server_it = servers_.find(server_fd);
		if (server_it != servers_.end())
		{
			server_it->second.active_connections--;
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

std::vector<std::string> PerServerConfigWebServer::getDetailedServerInfo() const
{
	std::vector<std::string> info;
	for (const auto &pair : servers_)
	{
		const auto &server = pair.second;
		std::ostringstream oss;
		oss << server.config.name << " (:" << server.config.port << ") - "
			<< "Active: " << server.active_connections << "/" << server.config.max_connections
			<< ", Requests: " << server.total_requests
			<< ", Rejected: " << server.rejected_connections
			<< ", Timeout: " << server.config.keep_alive_timeout << "s";
		info.push_back(oss.str());
	}
	return info;
}

void PerServerConfigWebServer::logMessage(const ServerConfig &config, const std::string &message)
{
	if (config.enable_logging)
	{
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);

		std::cout << "[" << std::put_time(std::localtime(&time_t), "%H:%M:%S") << "] "
				  << "[" << config.name << "] " << message << std::endl;
	}
}

// ... (rest of the implementation - createAndBindServer, setupEpoll, etc. remain the same)

bool PerServerConfigWebServer::setupEpoll()
{
	epoll_fd_ = epoll_create1(0);
	if (epoll_fd_ < 0)
	{
		std::cerr << "Failed to create epoll: " << strerror(errno) << std::endl;
		return false;
	}
	return true;
}

bool PerServerConfigWebServer::createAndBindServer(const ServerConfig &config)
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

bool PerServerConfigWebServer::setNonBlocking(int fd)
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
// main.cpp - Demonstration
// =====================================================

int main()
{
	try
	{
		PerServerConfigWebServer server;

		// Add servers with different configurations
		server.addServer(8080, "0.0.0.0", "FastServer", 15, 500, true);		// 15s timeout, 500 max conn
		server.addServer(8443, "0.0.0.0", "SecureServer", 60, 200, true);	// 60s timeout, 200 max conn
		server.addServer(9000, "127.0.0.1", "AdminServer", 120, 10, false); // 120s timeout, 10 max conn, no logs
		server.addServer(8888, "0.0.0.0", "TestServer", 5, 1000, true);		// 5s timeout, 1000 max conn

		// Set global epoll configuration
		server.setGlobalMaxEvents(2000); // Handle up to 2000 events per epoll_wait
		server.setCleanupInterval(3);	 // Check timeouts every 3 seconds

		// Custom request handler that respects per-server settings
		server.setRequestHandler([](const std::string &method, const std::string &path,
									const ServerConfig &config) -> std::string
								 {
									 if (config.name == "AdminServer" && path.find("/admin") == 0)
									 {
										 return "<h1>Admin Panel</h1><p>Secure admin interface with " +
												std::to_string(config.keep_alive_timeout) + "s timeout</p>";
									 }
									 else if (config.name == "FastServer" && path == "/fast")
									 {
										 return "<h1>Fast Response</h1><p>Optimized for quick responses!</p>";
									 }
									 return ""; // Use default
								 });

		if (!server.start())
		{
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