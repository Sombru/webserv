#pragma once

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdexcept>
#include <map>
#include <stdlib.h>
#include <vector>
#include <cctype>
#include <sys/wait.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <poll.h>
#include <sys/epoll.h>
#include <dirent.h>
// #include "Client.hpp"
// #include "Config.hpp"
// #include "Logger.hpp"
// #include "ServerManager.hpp"
// #include "Socket.hpp"

struct HttpResponse;


std::string readFile(const std::string& path);
std::string intToString(int n);
std::string serialize(const HttpResponse& response);