#pragma once
#include "Config.hpp"
#include "ParserManager.hpp"

class ServerManager {
private:
    ServerConfig serverConfig;
    Socket serverSocket;

public:
    ServerManager(const ServerConfig& config)
        : serverConfig(config), serverSocket(config.port) {}

    void setup()
    {
        serverSocket.setup();
    }

    void run()
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
};

