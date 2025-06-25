#include "ParserManager.hpp"

ParserManager::ParserManager(ParseType type, const std::string& filename)
{
	switch(type)
	{
		case(CONFIG):
			config.parse(filename);
			break;
		case(RESPONSE):
			std::cout << "here is the rule for parsing a response" << std::endl;
			break;
		case(REQUEST):
			std::cout << "here is the rule for parsing a request" << std::endl;
			break;
	}
}