#include "include/Socket.hpp"
#include "include/Client.hpp"
#include "include/Webserv.hpp"
#include "include/ParserManager.hpp"
#include "Logger.hpp"
#include "Socket.hpp"
#include <csignal> // For signal()

Socket* global_socket_ptr = NULL;

void handle_sigint(int sig) {
	std::cout << "\nCaught signal " << sig << ", exiting...\n";
	if (global_socket_ptr) {
		global_socket_ptr->Socket::closeServerSocket(); // method added to Socket
	}
}
int main()
{
	try //setup
	{
		// TODO config
		// Parser::parseConfig()

		Socket webserv(PORT);

		global_socket_ptr = &webserv; // assign global pointer

		signal(SIGINT, handle_sigint); // hook CTRL+C
		webserv.setup();

		while (true)
		{
			// accept client
			Client client(webserv);
			client.getRequest();
			ParserManager pm;
			pm.parseRequest(client);
			pm.buildResponse(webserv, client);
			webserv.response(client);
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		Logger::error(e.what());
		return 1;
	}
	return 0;
}

// http://localhost:8080/
