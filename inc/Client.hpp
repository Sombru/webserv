#pragma once

#include "Webserv.hpp"
#include "Socket.hpp"

class Client
{
private:
	int fd; // The client-specific socket file descriptor, returned by accept(), in the Socket class.
	// Used for reading(receiving requests) and writing(sending responses).
	int request_size;
	std::string raw_request; // Stores the full HTTP request as a raw string
	
public:
	Client() {} ; // Default constructor. Not used in this context, but defined for completeness.
	Client(Socket& socket); // Calls acceptClient() from the passed-in Socket object.
	~Client(); // Automatically close the socket when the client is destroyed.

	int getClientFd() const; // Simple getter for the socket file descriptor.
	std::string getRaw_request() const; // Returns the full raw request string received from the client.
	
	ssize_t response(std::string response);
	void getRequest(); // Reads data sent by the client over the socket.
	// HttpRequest parseRequest();

};