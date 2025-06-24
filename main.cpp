#include "include/Socket.hpp"
#include "include/Client.hpp"

int main()
{
	Socket server(PORT);

	if (server.setup() == false)
	{
		return 1;
	}

	std::ifstream index("webpages/index.html");
	std::stringstream page;
	page << index.rdbuf();
	std::cout << page.str();
	while (true)
	{
		// possibly use try here
		Client client(server);

		std::string request = client.recievedRequest();
		std::cout << "Received request:\n"
				  << request << std::endl;
		client.sendMessage(page.str());
	}

	return 0;
}

// http://localhost:8080/
