#pragma once

#include "Webserv.hpp"

#define BADFILE "BAD"
#define EMPTY "EMPTY"

std::string readFile(const std::string &path);
std::string intToString(int n);