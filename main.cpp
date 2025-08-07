#include "Logger.hpp"
#include "Config.hpp"
#include "Webserv.hpp"
#include "Utils.hpp"

#define PATH "configs/default.conf"
std::string getWord(std::string &fileBuff, size_t &start);

int main()
{
	std::string fileBuff = readFile(PATH);
	DEBUG(ConfigParse::tokenize(fileBuff));


}