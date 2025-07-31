#include "Client.hpp"
#include "Socket.hpp"
#include "Logger.hpp"


// Calls acceptClient() from the passed-in Socket object.
// This waits for and accepts a new client connection.
// If it fails, throws exception

Client::Client(int client_fd, int requset_size)
{
	this->fd = client_fd;
	// sockaddr_in client_addr;
	// socklen_t addrlen = sizeof(client_addr);
	// this->fd = accept(poll_fd, (sockaddr*)&client_addr, &addrlen);
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
	// Logger::debug(res);
	return (send(this->fd, res.c_str(), res.size(), 0));
}

void Client::appentToRaw(const char* data, size_t len) {
	raw_request.append(data, len);
	request_size += len;

	if (!headers_parsed) {
		size_t pos = raw_request.find("\r\n\r\n");
		if (pos != std::string::npos) {
			headers_parsed = true;
			headers_end_pos = pos;
			content_length = extractContentLength(raw_request);
		}
	}
}

// will return the full body size only
size_t Client::extractTotalLength() const {
	return headers_end_pos + 4 + content_length;
}

// will be used to know when it's safe to parse the request.
size_t Client::extractContentLength(const std::string& request) {
	size_t pos = request.find("Content-Length:");
	if (pos == std::string::npos)
		return 0;
	
	pos += std::string("Content-Length:").length();
	while (pos < request.size() && isspace(request[pos]))
		++pos;
		
	size_t end = request.find("\r\n", pos);
	std::string len_str = request.substr(pos, end - pos);
	return static_cast<size_t>(std::atoi(len_str.c_str()));
}

bool Client::headersParsed() const {
	return headers_parsed;
}