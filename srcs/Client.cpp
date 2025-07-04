#include "Client.hpp"
#include "Socket.hpp"
#include "Logger.hpp"

Client::Client(Socket& socket)
: request_size(socket.getRequestSize())
{
	fd = socket.acceptClient();
	if (fd < 0)
	{
		throw std::runtime_error("Failed to accept client connection");
	}
}
// Calls acceptClient() from the passed-in Socket object.
// This waits for and accepts a new client connection.
// If it fails, throws exception

Client::~Client()
{
	if (fd < -1) // (_fd >= 0)
	{
		std::cout << "closing " << fd << '\n';
		close(fd);
	}
}

int Client::getClientFd() const
{
	return fd;
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
	char buffer[this->request_size];
	std::memset(buffer, 0, sizeof(buffer));

	read(this->fd, buffer, sizeof(buffer) - 1);
	// recv(this->fd, buffer, sizeof(buffer) - 1, 0);

	//std::cout << "Received request:\n" << buffer << std::endl;
	Logger::debug("Recieved a request and saved to buffer");
	this->raw_request = buffer;
}
// Reads data sent by the client over the socket.
// Read() fill a buffer
// Stores the result in a raw_request.