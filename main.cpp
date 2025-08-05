#include "Webserv.hpp"
#include "Config.hpp"
#include "Utils.hpp"
#include "Logger.hpp"
#include "ServerManager.hpp"

// #define CONFIG_PATH "servers/test.conf"
volatile sig_atomic_t g_sigint = 0;

void ft_signal_handler(int c)
{
	(void)c;
	g_sigint = 1;
	Logger::info("STOPPING THE SERVER...\n");
}

int main(int argc, char *argv[])
{
	
	if (argc < 2)
	{
		Logger::error("provide a config file");
		return 1;
	}
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, ft_signal_handler);
	try
	{
		std::vector<ServerConfig> config = Config::parseConfig(argv[1]);
		Logger::info("Parsing complete. Server count: " + intToString(config.size()));

		// debuging
		// for (size_t i = 0; i < config.size(); i++)
		// {
		// 	Logger::debug(config[i]);
		// }
		ServerManager webserv(config);
		webserv.setup();
		webserv.run();
	}
	catch (const std::exception &e)
	{
		Logger::error(std::string(e.what()));
		return 3;
	}
}