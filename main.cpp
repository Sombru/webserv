#include "include/Socket.hpp"
#include "include/Client.hpp"
#include "include/Webserv.hpp"
#include "include/ParserManager.hpp"
#include "Logger.hpp"
#include "Socket.hpp"
#include "ServerManager.hpp"

Socket* global_socket_ptr = NULL;
ServerManager* global_server_mgr = NULL;

void handle_sigint(int sig) {
	std::cout << "\nCaught signal " << sig << ", exitting...\n";
	if (global_server_mgr)
		global_server_mgr->stop(); // breaks run() loop
	if (global_socket_ptr)
		global_socket_ptr->shutdown(); // closes server socket
	if (global_server_mgr)
		global_server_mgr->shutdown_epoll(); //closes epoll fd
	exit(0);
}


int main()
{
	try
	{
		signal(SIGINT, handle_sigint);

		Socket webserv(PORT);
		global_socket_ptr = &webserv;

		ServerManager serverManager(webserv);
		global_server_mgr = &serverManager; // Assign the global pointer

		serverManager.run();
	}
	catch (const std::exception &e)
	{
		//std::cerr << "Error: " << e.what() << std::endl;
		Logger::error(e.what());
		return 1;
	}
	return 0;
}

// http://localhost:8080/
