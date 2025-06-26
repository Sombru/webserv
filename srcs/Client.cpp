#include "include/Client.hpp"

Client::Client(Socket &server)
{
	_fd = server.acceptClient();
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

std::string Client::getRawRequest() const
{
	return raw_request;
}

ssize_t Client::sendMessage(std::string response)
{
	return send(_fd, response.c_str(), response.length(), 0);
}

void Client::recievedRequest()
{
	char buffer[4096];
	std::memset(buffer, 0, sizeof(buffer));

	read(_fd, buffer, sizeof(buffer) - 1);

	std::cout << "Received request:\n"
			  << buffer << std::endl;
	this->raw_request = buffer;
}