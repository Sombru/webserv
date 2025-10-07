#include "Logger.hpp"
#include "Config.hpp"
#include "Webserv.hpp"
#include "Utils.hpp"
#include "ServerManager.hpp"
#include <sstream>

#define PATH "configs/default.conf"

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " <config_file_path>" << std::endl;
		return 1;
	}

	Config config(argv[1]);
	
	if (config.parseConfig() == -1)
	{
		ERROR("Failed to parse configuration");
		return 1;
	}
	
	if (config.validateConfig() == -1)
	{
		ERROR("Configuration validation failed");
		return 1;
	}	
	
	ServerManager webserv(config.config);
	if (webserv.setup() < 0)
		return 1;
	webserv.run();
	return 0;
}