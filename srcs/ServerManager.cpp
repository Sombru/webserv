#include "ServerManager.hpp"
#include "Logger.hpp"
#include "Client.hpp"
#include "HTTP.hpp"
#include "Webserv.hpp"
#include <set>

ServerManager::ServerManager(const std::vector<ServerConfig> &servers)
	: servers(servers), running(true)
{
	this->sockets.reserve(this->servers.size()); // reserve just enought space for every socket

	for (unsigned int i = 0; i < this->servers.size(); i++)
	{
		this->sockets.push_back(Socket(this->servers[i])); // init every socket for every server
	}
}

void ServerManager::setup()
{
	for (unsigned int i = 0; i < this->sockets.size(); i++)
		this->sockets[i].setup();
}

void ServerManager::run()
{
	std::vector<pollfd> poll_fds;

	std::map<int, Client> clients; // clients fds-> Client object
	// Set of listening server FDs to distinguish from client FDs
	std::set<int> listening_fds;

	// Map from server FD to index in 'servers[]'
	std::map<int, size_t> server_fd_to_index;

	// Map from client FD to index in 'servers[]'
	std::map<int, size_t> fd_to_server_index;

	// Adding all listening server sockets to poll_fds
	for (size_t i = 0; i < sockets.size(); ++i)
	{
		int server_fd = sockets[i].getServerFD();

		pollfd pfd;
		pfd.fd = server_fd;
		pfd.events = POLLIN; // wait for new connections
		poll_fds.push_back(pfd);

		listening_fds.insert(server_fd);
		server_fd_to_index[server_fd] = i; // remembering which index this server belongs to
	}

	// Polling loop
	while (running)
	{
		int ret = poll(&poll_fds[0], poll_fds.size(), -1);
		if (g_sigint)
		{
			Logger::info("Poll interrupted by SIGINT. Shutting down ...");
			break;
		}
		if (ret < 0)
		{
			// if poll failes unknown
			Logger::error("poll() failed unexpectedly");
			break;
		}
		for (size_t i = 0; i < poll_fds.size(); ++i)
		{
			int fd = poll_fds[i].fd;

			if (!(poll_fds[i].revents & POLLIN))
				continue;

			// Check if this is a listening socket
			if (listening_fds.count(fd))
			{
				int client_fd = accept(fd, NULL, NULL);
				if (client_fd < 0)
				{
					Logger::error("Failed to accept client: " + std::string(strerror(errno)));
					continue;
				}

				Logger::info("Accepted new client: fd " + intToString(client_fd));

				// Map client_fd to the correct server
				std::map<int, size_t>::iterator server_it = server_fd_to_index.find(fd);
				if (server_it == server_fd_to_index.end())
				{
					Logger::error("Server FD not found in map");
					close(client_fd);
					continue;
				}
				fd_to_server_index[client_fd] = server_it->second;

				// Add new client to poll
				struct pollfd client_pollfd;
				client_pollfd.fd = client_fd;
				client_pollfd.events = POLLIN;
				poll_fds.push_back(client_pollfd);
				continue;
			}

			// Not a listening socket — handle client
			if (clients.find(fd) == clients.end())
			{
				clients[fd] = Client(fd, 8192);
			}
			Client &client = clients[fd];

			char buffer[8192];
			ssize_t n = recv(fd, buffer, sizeof(buffer), 0);
			if (n <= 0)
			{
				if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
					continue;

				Logger::info((n == 0 ? "Client disconnected" : "recv() failed") +
							 std::string(" on fd ") + intToString(fd) + ": " + strerror(errno));

				close(fd);
				poll_fds.erase(poll_fds.begin() + i);
				clients.erase(fd);
				fd_to_server_index.erase(fd);
				--i;
				continue;
			}

			client.appentToRaw(buffer, n);
			Logger::debug("Buffer size after append: " + intToString(client.getRaw_request().size()));
			Logger::debug("Expected total length: " + intToString(client.extractTotalLength()));

			if (client.headersParsed() &&
				client.getRaw_request().size() >= client.extractTotalLength())
			{
				std::map<int, size_t>::iterator server_it = fd_to_server_index.find(fd);
				if (server_it == fd_to_server_index.end())
				{
					Logger::error("Server index not found for client FD");
					continue;
				}

				HttpRequest request = parseRequset(client.getRaw_request(), servers[server_it->second]);
				HttpResponse response = generateResponse(request, servers[server_it->second]);
				client.sendResponse(response);

				Logger::debug(request);

				close(fd);
				poll_fds.erase(poll_fds.begin() + i);
				clients.erase(fd);
				fd_to_server_index.erase(fd);
				--i;
			}
		}
	}
}

ServerManager::~ServerManager()
{
	Logger::info("Shutting down webserver");
}
