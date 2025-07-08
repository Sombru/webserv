#include "Client.hpp"
//#include "Socket.hpp"
#include "Logger.hpp"

// Client::Client(Socket& socket)
// : request_size(socket.getRequestSize())
// {
// 	fd = socket.acceptClient();
// 	if (fd < 0)
// 	{
// 		throw std::runtime_error("Failed to accept client connection");
// 	}
// }
// // Calls acceptClient() from the passed-in Socket object.
// // This waits for and accepts a new client connection.
// // If it fails, throws exception

// Client::~Client()
// {
// 	if (fd < -1) // (_fd >= 0)
// 	{
// 		std::cout << "closing " << fd << '\n';
// 		close(fd);
// 	}
// }

// int Client::getClientFd() const
// {
// 	return fd;
// }
// // Getter for the socket FD.

// std::string Client::getRaw_request() const
// {
// 	return this->raw_request;
// }
// // Returns the full raw request string received from the client.
// // Useful later when you parse the request.

// void Client::getRequest()
// {
// 	char buffer[this->request_size];
// 	std::memset(buffer, 0, sizeof(buffer));

// 	read(this->fd, buffer, sizeof(buffer) - 1);
// 	// recv(this->fd, buffer, sizeof(buffer) - 1, 0);

// 	//std::cout << "Received request:\n" << buffer << std::endl;
// 	Logger::debug("Recieved a request and saved to buffer");
// 	this->raw_request = buffer;
// }
// // Reads data sent by the client over the socket.
// // Read() fill a buffer
// // Stores the result in a raw_request.



// Client::Client(int fd, const ServerConfig& config)
//     :fd(fd), config(config) {}

Client::~Client() {
    if (fd != -1) {
        close(fd);
        Logger::info("Closed client fd: " + intToString(fd));
    }
}

Client::Client(const ServerConfig& config) 
    : fd(-1), request_size (config.client_max_body_size), raw_request(""), serverConfig(config) {}

void Client::setClientFd(int fd) {
    this->fd = fd;
}

const ServerConfig& Client::getServerConfig() const {
    return this->serverConfig;
}

void Client::setClientFd(int fd) {
    this->fd = fd;
}

int Client::getClientFd() const {
    return fd;
}

std::string Client::getRaw_request() const {
    return raw_request;
}

void Client::receiveRequest() {
    std::vector<char> buffer(request_size); // using config-based max size instead of hardcoding 8192
    std::memset(&buffer, 0, sizeof(buffer));

    ssize_t bytes = read(fd, &buffer[0], buffer.size() - 1);
    if (bytes <= 0) {
        throw std::runtime_error("Client disconnected or read failed");
    }
    raw_request.assign(buffer.begin(), buffer.begin() + bytes);
    Logger::debug("Received request:\n" + raw_request);
}

ssize_t Client::sendResponse(const std::string& response) {
    return send(fd, response.c_str(), response.size(), 0);
}
