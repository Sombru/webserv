#include "include/Socket.hpp"
#include "include/Client.hpp"
#include "include/Webserv.hpp"
#include "include/Get.hpp"
#include "Logger.hpp"

int runServer(Config config)
{
	std::cout << config.server.port;
	Socket server(config.server.port);
	if (server.setup() == false)
		throw std::runtime_error("Failed to set up server");

	while (true)
	{
		Client client(server);
		client.recievedRequest();
		ParserManager pm(REQUEST, client.getRawRequest());
		std::cout << pm.request;
		if (pm.request.method == "GET")
			client.sendMessage(Get(pm.request).response());
	}
	return 0;
}

int main()
{
	try
	{
		ParserManager pm(CONFIG, "config/default.conf");
		std::cout << pm.config;
		//runServer(pm.config);
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
