#include "Client.hpp"

Client::Client() {}

Client::Client(Server *srv)
	: server(srv), lastActivity(time(NULL))
{

}

