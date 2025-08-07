#include "Logger.hpp"
#include "Config.hpp"
#include "Webserv.hpp"
#include "Utils.hpp"

#define PATH "configs/default.conf"

int main()
{
	std::string fileBuff = readFile(PATH);
	std::vector<Token> tokens = Config::tokenize(fileBuff);

	size_t i = 0;
	ServerConfig serv = Config::parseServerConfig(tokens, i)

}