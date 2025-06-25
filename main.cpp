#include "include/Socket.hpp"
#include "include/Client.hpp"
#include "include/Webserv.hpp"
#include "include/Get.hpp"
#include "Logger.hpp"

int runServer(std::map<std::string, std::string> &config)
{
	int port = std::atoi(config["port"].c_str());
	Socket server(port);
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

std::map<std::string, std::string> storeConfig()
{
	std::string line;
	std::ifstream index;
	openFile(index, "config/default.conf");
	std::map<std::string, std::string> config;
	while (std::getline(index, line))
	{
		std::stringstream ss(line);
		std::string date, value;

		if (std::getline(ss, date, ' ') && std::getline(ss, value))
		{
			if (!date.empty() && !value.empty())
				config.insert(std::make_pair(date, value));
		}
	}
	return config;
}

int main()
{
	try
	{
		std::map<std::string, std::string> config = storeConfig();
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
