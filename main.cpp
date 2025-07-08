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

// TODO serv_manager shold have setup int separate function not in constructor
// implement minimal server loop
// more logging
// before porting com_manager have it working in a minimal way pls (just static page)
// port com manager together to not make everyone lose their mind
// cgi/serving pages later together

// tip: logger if now better it can accepr any of our structure