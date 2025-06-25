#include "include/Socket.hpp"
#include "include/Client.hpp"
#include "include/Libraries.hpp"

int runServer(std::map<std::string, std::string> &config)
{
	int port = std::atoi(config["port"].c_str());
	Socket server(port);
	if (server.setup() == false)
		throw std::runtime_error("Failed to set up server");

	std::ifstream index;
	openFile(index, "webpages/index.html");

	std::stringstream page;
	page << index.rdbuf();
	std::string body = page.str();
	std::stringstream response;
	response << "HTTP/1.1 200 OK\r\n"
			 << "Content-Type: text/html\r\n"
			 << "Content-Length: " << body.size() << "\r\n"
			 << "\r\n"
			 << body;
	std::string resource = response.str();
	std::cout << resource << '\n';
	while (true)
	{
		Client client(server);
		std::string request = client.recievedRequest();
		std::cout << "Received request:\n"
				  << request << std::endl;
		if (client.sendMessage(resource) < 0)
			std::cerr << "ERROR MESSAGE\n";
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
		return 1;
	}
	return 0;
}

// http://localhost:8080/
