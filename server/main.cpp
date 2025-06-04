#include "Socket.hpp"

int main()
{
	Socket server(PORT);

	if (server.setup() == false)
	{
		return 1;
	}

	while (true)
	{
		int client_fd = server.acceptClient();
		if (client_fd < 0)
		{
			std::cerr << "Failed to accept client\n";
			continue;
		}

		char buffer[4096];
		std::memset(buffer, 0, sizeof(buffer));
		read(client_fd, buffer, sizeof(buffer) - 1);
		std::cout << "Received request:\n"
				  << buffer << std::endl;

		const char *response =
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/plain\r\n"
			"Content-Length: 13\r\n"
			"\r\n"
			"Hello, world!";

		send(client_fd, response, std::strlen(response), 0);
		close(client_fd);
	}

	return 0;
}

// http://localhost:8080/