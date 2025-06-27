#pragma once
#include "include/Webserv.hpp"
#include "Client.hpp"
#include "Socket.hpp"

// enum ParseType
// {
//     CONFIG,
//     RESPONSE,
//     REQUEST,
// };

class ParserManager 
{
public:
    Config config;
	HttpRequest request;
	HttpResponse response;

	ParserManager() {};
    // ParserManager(ParseType type, const std::string& filename);
	int parseRequest(const Client& client);
	int buildResponse(Socket& webserv, const Client& client);

};