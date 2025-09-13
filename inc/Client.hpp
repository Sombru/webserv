#pragma once

#include "Logger.hpp"
#include "Webserv.hpp"
#include "Utils.hpp"
#include "Server.hpp"
#include <ctime>

class Server;

class Client 
{
public:
	Server *server;
	time_t lastActivity;
	
	Client();
	Client(Server *srv);
};

