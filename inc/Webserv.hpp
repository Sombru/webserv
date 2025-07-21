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
#include <csignal>
// #include "Client.hpp"
// #include "Config.hpp"
// #include "Logger.hpp"
// #include "ServerManager.hpp"
// #include "Socket.hpp"

extern volatile sig_atomic_t g_sigint;

struct HttpResponse;

std::string readFile(const std::string& path);
std::string intToString(int n);
std::string serialize(const HttpResponse& response);
std::vector<std::string> getLocationContents(const std::string& server_root, const std::string& location_root);
std::vector<std::string> getLocationContents(const std::string& path);
std::string getFileExtension(const std::string& file);