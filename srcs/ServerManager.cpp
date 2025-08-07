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

	// Set of listening server FDs to distinguish from client FDs
	std::set<int> listening_fds;

	// Map from server FD to index in 'servers[]'
	std::map<int, size_t> server_fd_to_index;

	// Map from client FD to index in 'servers[]'
	std::map<int, size_t> fd_to_server_index;

	// Map of client fds to clients, in case we want to record big files
	std::map<int, Client*> clients;

	// Adding all listening server sockets to poll_fds
	for (size_t i = 0; i < sockets.size(); ++i) {
		int server_fd = sockets[i].getServerFD();

		pollfd pfd;
		pfd.fd = server_fd;
		pfd.events = POLLIN; // wait for new connections
		poll_fds.push_back(pfd);

		listening_fds.insert(server_fd);
		server_fd_to_index[server_fd] = i; //reminbering which index this server belongs to
	}

	// Polling loop
	while (running) {
		int ret = poll(&poll_fds[0], poll_fds.size(), -1);
		if (g_sigint) {
			Logger::info("Poll interrupted by SIGINT. Shutting down ...");
			break;
		}
		if (ret < 0) {
			// if poll failes unknown
			Logger::error("poll() failed unexpectedly");
			break;
		}
		for (size_t i = 0; i < poll_fds.size(); ++i)
		{
			int fd = poll_fds[i].fd;
			
			if (!(poll_fds[i].revents & POLLIN))
				continue; // nothing to read, skip
			
			// if this is a listening socket: accept new client
			if (listening_fds.count(fd)) {
				int client_fd = accept(fd, NULL, NULL);
				if (client_fd < 0) {
					Logger::error("Failed to accept client");
					continue;
				}

				// find which server this FD belongs to
				size_t server_index = server_fd_to_index[fd];

				// SAve client_fd -> server_index mapping
				fd_to_server_index[client_fd] = server_index;

				// add client to poll_fds
				pollfd client_pfd;
				client_pfd.fd = client_fd;
				client_pfd.events = POLLIN;
				poll_fds.push_back(client_pfd);

				Client* client = new Client(client_fd, 10485760);
				clients.insert(std::pair<int, Client*>(client_fd, client));

				Logger::info("Accepted new client: fd " + intToString(client_fd));
				continue; // skip further processing of this fd
			}

			// Map fd to correct server
			if (fd_to_server_index.find(fd) == fd_to_server_index.end()) {
				Logger::error("Could not find server index for fd " + intToString(fd));
				continue;
			}
			size_t server_index = fd_to_server_index[fd];

			// Otherwise: client is sending us a request
			Client* client = clients.at(fd);
			client->makeRequest();

			// Parse and handle the request
			HttpRequest request = parseRequset(client->getRaw_request(), this->servers[server_index]);

			size_t len = request.body.find('\0');
			if (len == std::string::npos) {
				len = request.body.length();
			}
			
			if (len < (size_t)std::atoi(request.headers["Content-Length"].c_str())) {
				continue;
			}

			Logger::debug("===RAW REQUEST START ===");

			Logger::debug("Request is valid");
			// TODO is valid request
			// switch to GET POST DELTE in this section to minimze err cheks
			// have a map of headers that are handled
			HttpResponse response = generateResponse(request, servers[server_index]);

			client->sendResponse(response);
			free(client);
			clients.erase(fd);
			Logger::debug(request);
			close(fd);
			poll_fds.erase(poll_fds.begin() + i);
			fd_to_server_index.erase(fd);
			--i;
		}
	}
}

ServerManager::~ServerManager()
{
	Logger::info("Shutting down webserver");
}
