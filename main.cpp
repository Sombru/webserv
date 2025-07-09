#include "Webserv.hpp"
#include "Config.hpp"
#include "ServerManager.hpp"
#include "Logger.hpp"
#include "Client.hpp"

int parseConfig(const std::string& configPath, std::vector<ServerConfig>& servers)
{
	std::vector<Token> tokens = tokenize_config(openFile(configPath));

	// Logger::debug(tokens);
	size_t i = 0;
	try
	{
		while (i < tokens.size())
		{
			if (tokens[i].type == WORD && tokens[i].value == "server")
			{
				i++;
				ServerConfig srv;
				parse_server(srv, tokens, i);
				servers.push_back(srv);
			}
			else
				throw std::runtime_error("Expected 'server' block");
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
		return (-1);
	}
	return (0);

}

int main()
{
	std::vector<ServerConfig> servers;

	if (parseConfig("servers/default.conf", servers) == -1)
	{
		return (1);
	}
	try
	{
		ServerManager webserv(servers);
		webserv.setup();
		webserv.run();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return (1);
	}
	

	// Socket socket(servers[0]);
	// socket.setup();
}

// setup poll/epoll
// memory?
// do some requests and responses