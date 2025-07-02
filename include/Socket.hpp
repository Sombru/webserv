#pragma once

#include "Webserv.hpp"
#include "Client.hpp"

#define PORT 8082

class Client;
class Socket;

extern Socket* global_socket_ptr;

class Socket
{
private:
	int _server_fd; // File descriptor returned by socket()
	int _port;		// Port number where the server instance will listen for incoming connections.
	struct sockaddr_in _address; // holds the server IP, port, and family(AF_INET).
	socklen_t _addrlen; // is needed for accept() - it stores the size of the sockaddr_in.

	std::string res; // the response that the server will send back to the client (stored as a string).

public:
	Socket() {}; // Default constructor(probably unused if we're always passing a port).
	Socket(int port); //Initializes the socket instance.
	~Socket(); // Destructor ensures cleanup: if the server socket is open, it's closed.

	bool isrunning; // Simple flag to keep track of server state.

	bool setup(); // Sets up the entire listening socket. socket(), setsockopt(), bind(), listen().
	int acceptClient(); // Waits for a client to connect. Returns a new file desctiptor - used to communicate with that specific client.
	int response(const Client& client); // Sends the current response string - res, to a specific client.

	int getServerFd() const; // Getter for the main listening socket FD.
	void setResponse(std::string res); // Assigns a response string to the res field
	void closeServerSocket();
	void shutdown();
};