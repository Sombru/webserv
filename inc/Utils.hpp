#pragma once
#include "Webserv.hpp"

#define BADFILE "BADFILE"
std::string readFile(const std::string &path);

std::string intToString(int n);
std::string getTimestamp();

// HTTP parsing utilities
std::map<std::string, std::string> parseQueryString(const std::string &queryString);

std::string getMimeType(const std::string &path);
std::string readFileBinary(const std::string &path);