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

// #include "Client.hpp"
// #include "Config.hpp"
// #include "Logger.hpp"
// #include "ServerManager.hpp"
// #include "Socket.hpp"

std::string openFile(const std::string& path);
std::string intToString(int n);