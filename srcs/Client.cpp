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
	this->fd = accept(poll_fd, (sockaddr *)&client_addr, &addrlen);
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

	// std::cout << "Received request:\n" << buffer << std::endl;
	Logger::debug("Recieved a request and saved to buffer");
	this->raw_request = buffer;
}
// Reads data sent by the client over the socket.
// Read() fill a buffer
// Stores the result in a raw_request.

// uses the struct request to check the request and execute it
HttpResponse recieveResponse(HttpRequest &request)
{
	HttpResponse responceRecieved;
	std::string fullPath;

	// if (config.root.c_str() == "www/portfolio/" &&
	// 	config.index.c_str() == "index.html")
	// {

	if (request.headers["Host"] == "127.0.0.3:8080")
	{
		//fullPath = request.path;		  // path directory
		//fullPath += request.query_string; // adding index directory
		fullPath = "www/portfolio/index.html";
		std::ifstream file(fullPath.c_str());
		Logger::debug("working");
		if (file.is_open())
		{
				std::stringstream body_stream;
				body_stream << file.rdbuf();
				responceRecieved.version = request.version;
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

		// else
		// {
		// 	std::cout << "Not ready yet\n";
		// 	// run error page
		// }
	}
	// else
	//{ // error opening file -> open page error
	// }
	// Logger::debug("Not working yet");
	return (responceRecieved);
}

// sending the response back to the client - serializes the HTTP response
std::string serialize(HttpResponse &response){
	std::ostringstream oss;

	oss << response.version << " " << response.status_code << " "
		<< response.status_text << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it = response.headers.begin(); it != response.headers.end(); ++it) {
		oss << it->first <<": "<< it->second << "\r\n";
	}
	oss << "\r\n";
	oss << response.body;

	return (oss.str());
}