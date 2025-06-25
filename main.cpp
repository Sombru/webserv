#include "include/Socket.hpp"
#include "include/Client.hpp"
#include "include/Webserv.hpp"
#include "include/Get.hpp"
#include "Logger.hpp"

int runServer(Config config)
{
	Socket server(config.server.port);
	if (server.setup() == false)
		throw std::runtime_error("Failed to set up server");

	while (true)
	{
		Client client(server);
		client.recievedRequest();
		HttpRequest req(client.parseRequest());

		std::cout << req;
		if (req.method == "GET")
			client.sendMessage(Get(req).response());
	}
	return 0;
}

int main()
{
	try
	{
		Config config;
		config.loadFromFile("config/default.conf");
		runServer(config);
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		Logger::error(e.what());
		return 1;
	}
	return 0;
}

// http://localhost:8080/
