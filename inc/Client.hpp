#pragma once

#include "Webserv.hpp"
#include "Socket.hpp"

class Client
{
private:
	int fd; // The client-specific socket file descriptor, returned by accept(), in the Socket class.
	// Used for reading(receiving requests) and writing(sending responses).
	size_t request_size;
	std::string raw_request; // Stores the full HTTP request as a raw string
	
	// all below needed for handling long requests
	bool headers_parsed;
	size_t content_length;
	size_t headers_end_pos;

public:
	Client() {} ; // Default constructor. Not used in this context, but defined for completeness.
	Client(Socket& socket); // Calls acceptClient() from the passed-in Socket object.
	Client(int poll_fd, int requset_size);
	~Client(); // Automatically close the socket when the client is destroyed.

	int getClientFd() const; // Simple getter for the socket file descriptor.
	std::string getRaw_request() const; // Returns the full raw request string received from the client.
	void appentToRaw(const char* data, size_t len); // appending the request, so that we get the whole request even if it comes in chunks
	bool isReqiestComplete() const; // checking if we have the whole request transfered before continuing with the execution of the request
	size_t extractContentLength(const std::string& request); // will extract the total length of the body only
	size_t extractTotalLength() const; // will know the total data the server should recieve before calling the parse();

	bool headersParsed() const;
	void makeRequest(); // Reads data sent by the client over the socket.

	ssize_t sendResponse(const HttpResponse& response);
	ssize_t sendResponse(const std::string& response);

};