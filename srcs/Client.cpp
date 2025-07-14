#include "Client.hpp"
#include "Socket.hpp"
#include "Logger.hpp"


// Calls acceptClient() from the passed-in Socket object.
// This waits for and accepts a new client connection.
// If it fails, throws exception

Client::Client(int poll_fd, int requset_size)
{
	sockaddr_in client_addr;
	socklen_t addrlen = sizeof(client_addr);
	this->fd = accept(poll_fd, (sockaddr*)&client_addr, &addrlen);
	this->request_size = requset_size;
}

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

void Client::makeRequest()
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

ssize_t Client::sendResponse(const std::string& response)
{
	return (send(this->fd, response.c_str(), this->request_size, 0));
}

ssize_t Client::sendResponse(const HttpResponse& response)
{
	std::string res = serialize(response);
	return (send(this->fd, res.c_str(), this->request_size, 0));
}