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

Client::~Client()
{
	if (_fd < -1)
	{
		std::cout << "closing " << _fd << '\n';
		close(_fd);
	}
}

int Client::getClientFd() const
{
	return _fd;
}

std::string Client::getRaw_request() const
{
	return this->raw_request;
}

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

