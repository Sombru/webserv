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
	close(fd);
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
	this->request_size_fail = true;
	char buffer[this->request_size];
	std::memset(buffer, 0, sizeof(buffer));

	ssize_t bytes = read(this->fd, buffer, sizeof(buffer) - 1);
	if (bytes <= 0) {
		throw std::runtime_error("Client disconnected or read failed");
	}
	// recv(this->fd, buffer, sizeof(buffer) - 1, 0);

	// ⚠️ check if request body exceeds limit (e.g. from config file)
	if (bytes > this->request_size) {
		Logger::error("Error. Request body too large");
		request_size_fail = false;
		return ;
	}
	//std::cout << "Received request:\n" << buffer << std::endl;
	Logger::debug("Recieved a request and saved to buffer");
	this->raw_request = buffer;
}
// Reads data sent by the client over the socket.
// Read() fill a buffer
// Stores the result in a raw_request.

bool Client::get_request_size_fail(void) {
	return (this->request_size_fail);
}