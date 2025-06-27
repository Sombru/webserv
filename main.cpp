#include "include/Socket.hpp"
#include "include/Client.hpp"
#include "include/Webserv.hpp"
#include "include/ParserManager.hpp"
#include "Logger.hpp"
#include "Config.hpp"

int main()
{
	try //setup
	{
		// TODO config
		// Parser::parseConfig()

		std::ifstream file("config/default.conf");
		std::stringstream buffer;
		buffer << file.rdbuf();

		std::vector<Token> tokens = tokenize_config(buffer.str());
		// std::cout << tokens;
		
		ServerConfig serv;
		size_t i = 1;
		parse_server(serv, tokens, i);
		std::cout << serv << '\n';

		Socket webserv(PORT);
		webserv.setup();

		while (true)
		{
			// accept client
			Client client(webserv);
			client.getRequest();
			ParserManager pm;
			pm.parseRequest(client);
			pm.buildResponse(webserv);
			webserv.response(client);
		}
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
