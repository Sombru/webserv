#include "Logger.hpp"
#include "Config.hpp"
#include "Webserv.hpp"
#include "Utils.hpp"
#include "ServerManager.hpp"
#include <sstream>

#define PATH "configs/default.conf"

int main()
{
	Config config(const_cast<char*>(PATH));
	
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
	
	// DEBUG(config);
	ServerManager webserv(config.config);
	if (webserv.setup() < 0)
		return 1;
	webserv.run();
	// DEBUG(address);
	// DEBUG(port);
	return 0;
}