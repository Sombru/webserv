#pragma once

#include "Webserv.hpp"
#include "Config.hpp"

struct ServerConfig;
class Client;

class Socket
{
private:
	int server_fd;
	int port;
	std::string addres_str;
	struct addrinfo address;
	socklen_t addrlen;
	int request_size;

public:
	bool running;

	Socket(const ServerConfig &config);
	~Socket();

	bool setup();
	int acceptClient(int poll_fd, sockaddr *addr, socklen_t *addrlen);
	int respond(const Client &client);

	int getServerFD() const;
	int getRequestSize() const;

	void closeServerSocket();
};
