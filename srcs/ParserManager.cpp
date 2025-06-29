#include "ParserManager.hpp"
#include "Client.hpp"
#include "Logger.hpp"
#include "Config.hpp"
#include "Webserv.hpp"

ServerConfig& ParserManager::getServerConfig()
{
	return this->config;
}

std::vector<Token>& ParserManager::getTokens()
{
	return this->tokens;
}

