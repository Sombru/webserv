#pragma once
#include "include/Webserv.hpp"
#include "Client.hpp"
#include "Socket.hpp"

class ParserManager 
{
public:
    // Config config;
	HttpRequest request;
	HttpResponse response;

	ParserManager() {};
	int parseRequest(const Client& client);
	int buildResponse(Socket& socket);

};