#include "include/Client.hpp"
#include "Logger.hpp"

Client::Client(Socket& webserv)
{
	_fd = webserv.acceptClient();
	if (_fd < 0)
	{
		throw std::runtime_error("Failed to accept client connection");
	}
}
// Calls acceptClient() from the passed-in Socket object.
// This waits for and accepts a new client connection.
// If it fails, throws exception

Client::~Client()
{
	if (_fd < -1) // (_fd >= 0)
	{
		std::cout << "closing " << _fd << '\n';
		close(_fd);
	}
}

int Client::getClientFd() const
{
	return _fd;
}
// Getter for the socket FD.

std::string Client::getRaw_request() const
{
	return this->raw_request;
}
// Returns the full raw request string received from the client.
// Useful later when you parse the request.

void Client::getRequest()
{
	char buffer[4096];
	std::memset(buffer, 0, sizeof(buffer));

	read(this->_fd, buffer, sizeof(buffer) - 1);
	// recv(this->_fd, buffer, sizeof(buffer) - 1, 0);

	std::cout << "Received request:\n"
			  << buffer << std::endl;
	// Logger::debug("recieved a request");
	this->raw_request = buffer;
}
// Reads data sent by the client over the socket.
// Read() fill a buffer
// Stores the result in a raw_request.

