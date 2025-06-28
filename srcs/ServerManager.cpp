#include "ServerManager.hpp"
#include "include/Socket.hpp"

void ServerManager::setup()
{
	serverSocket.setup();
}

void ServerManager::run()
{
	while (true)
	{
		Client client(serverSocket);
		client.getRequest();

		ParserManager pm;
		pm.parseRequest(client);
		pm.buildResponse(serverSocket);

		serverSocket.response(client);
	}
}