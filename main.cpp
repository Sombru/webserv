#include "Webserv.hpp"
#include "Config.hpp"
#include "Utils.hpp"
#include "Logger.hpp"

#define CONFIG_PATH "servers/test.conf"

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		Logger::error("provide a config file");
		return 1;
	}
	try
	{
		std::vector<ServerConfig> config = Config::parseConfig(argv[1]);
		Logger::info("Parsing complete. Server count: " + intToString(config.size()));

		// debuging
		for (size_t i = 0; i < config.size(); i++)
		{
			Logger::debug(config[i]);
		}
		// server manager here 
	}
	catch (const std::exception &e)
	{
		Logger::error("Parsing failed: " + std::string(e.what()));
		return 3;
	}
}