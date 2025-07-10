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

Token_response addTokenResp(TokenTypeRes type, std::string value) {
	Token_response token;

	token.type = type;
	token.value = value;
	return token;
}

// tokenizing the request
std::vector<Token_response> tokenize_request(std::string rawResponse) {
	std::vector<Token_response> tokens;
	std::string current;
	char c;
	for (size_t i = 0; i < rawResponse.length(); i++) {
		c = rawResponse[i];
		if (isspace(c)) {
			if (!current.empty()) {
				tokens.push_back(addTokenResp(WORD, current));
				current.clear();
			}
		} else if (c == ':') {
			if (!current.empty()) {
				tokens.push_back(addTokenResp(WORD, current));
				current.clear();
			}
			tokens.push_back(addTokenResp(COLON, ":"));
		} else {
			current += c;
		}
	}
	if (!current.empty()) {
		tokens.push_back(addTokenResp(WORD, current));
	}
	return tokens;
}
// takes the data from raw_request - fills the response struct and returns response
ssize_t Client::response() {
	std::vector<Token_response> tokens;
	tokens = tokenize_request(this->raw_request);
	Logger::debug("Raw request tokrnized");
	HttpResponse responseRecieved = recieveResponse()


} // uses the raw request to send response


HttpResponse recieveResponse(HttpRequest rawRequest, const ServerConfig &config)
{
	
	HttpResponse responceRecieved;
	std::string fullPath;

	// if (config.root.c_str() == "www/portfolio/" &&
	// 	config.index.c_str() == "index.html")
	// {
	fullPath = config.root;	  // path directory
	fullPath += config.index; // adding index directory
	std::ifstream file(fullPath.c_str());

	Logger::debug("working");
	if (file.is_open())
	{
		if (config.root == "www/portfolio/" &&
			config.index == "index.html")
		{
			std::stringstream body_stream;
			body_stream << file.rdbuf();
			responceRecieved.body = body_stream.str();
			responceRecieved.status_code = 200;
			responceRecieved.status_text = "OK";
			responceRecieved.headers["Content-Type"] = "text/html";
			{
				Logger::info("Proper page found\n");
				std::ostringstream len_stream;
				len_stream << responceRecieved.body.length();
				responceRecieved.headers["Content-Length"] = len_stream.str();
				return (responceRecieved);
			}
		}
		// if () faila otgovarq na neshto poznato neka se otvori, inache error

		else
		{
			std::cout << "Not ready yet\n";
			// run error page
		}
	}
	//else
	//{ // error opening file -> open page error
	//}
	Logger::debug("Not working yet");
	return (responceRecieved);
}