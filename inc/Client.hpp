#pragma once

#include "Webserv.hpp"
#include "Socket.hpp"

class Client
{
private:
	int fd; // The client-specific socket file descriptor, returned by accept(), in the Socket class.
	// Used for reading(receiving requests) and writing(sending responses).
	long request_size;
	std::string raw_request; // Stores the full HTTP request as a raw string
	
public:
	Client() {} ; // Default constructor. Not used in this context, but defined for completeness.
	Client(Socket& socket); // Calls acceptClient() from the passed-in Socket object.
	Client(int poll_fd, long requset_size);
	~Client(); // Automatically close the socket when the client is destroyed.

	int getClientFd() const; // Simple getter for the socket file descriptor.
	std::string getRaw_request() const; // Returns the full raw request string received from the client.
	
	void makeRequest(); // Reads data sent by the client over the socket.

	ssize_t sendResponse(const HttpResponse& response);
	ssize_t sendResponse(const std::string& response);

};