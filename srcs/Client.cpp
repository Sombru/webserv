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

ssize_t Client::sendMessage(std::string response)
{
	return send(_fd, response.c_str(), response.length(), 0);
}

void Client::recievedRequest()
{
	char buffer[4096];
	std::memset(buffer, 0, sizeof(buffer));

	read(_fd, buffer, sizeof(buffer) - 1);

	// std::cout << "Received request:\n"
			//   << buffer << std::endl;
	this->raw_request = buffer;
}

HttpRequest Client::parseRequest()
{
	HttpRequest req;
	std::istringstream stream(this->raw_request);
	std::string line;

	// Parse the request line: "GET /path?query=string HTTP/1.1"
	if (std::getline(stream, line))
	{
		std::istringstream line_stream(line);
		line_stream >> req.method;

		std::string full_path;
		line_stream >> full_path;

		size_t q = full_path.find('?');
		if (q != std::string::npos)
		{
			req.path = full_path.substr(0, q);
			req.query_string = full_path.substr(q + 1);
			req.query_params = parse_query(req.query_string);
		}
		else
		{
			req.path = full_path;
		}

		line_stream >> req.version;
	}

	while (std::getline(stream, line) && line != "\r")
	{
		size_t colon = line.find(':');
		if (colon != std::string::npos)
		{
			std::string key = line.substr(0, colon);
			std::string value = line.substr(colon + 1);
			if (!value.empty() && value[0] == ' ')
				value = value.substr(1);
			if (!value.empty() && value[value.length() - 1] == '\r')
				value = value.substr(0, value.length() - 1);
			req.headers[key] = value;
		}
	}
	// req.path = INDEX; // this will be something else idk what for now
	return (req);
}