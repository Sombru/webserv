#pragma once
#include "include/Webserv.hpp"
#include "Client.hpp"
#include "Socket.hpp"
#include "Config.hpp"

enum ParseType
{
    CONFIG,
    RESPONSE,
    REQUEST,
};

class ParserManager 
{
public:
    ServerConfig config;
	HttpRequest request;
	HttpResponse response;
	std::vector<Token> tokens;

	ParserManager() {};

	ServerConfig& getServerConfig ();
	std::vector<Token>& getTokens();

	int parseConfig(const std::string &path);
	int parseRequest(const Client& client);
	int buildResponse(Socket& socket);
};