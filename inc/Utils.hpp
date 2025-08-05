#pragma once

#include "Webserv.hpp"
#include "HTTP.hpp"

#define BADFILE "BAD"
#define EMPTY "EMPTY"

std::string readFile(const std::string &path);
std::string intToString(int n);
std::string serialize(const HttpResponse &);
bool is_directory(const std::string &path);
std::vector<std::string> getDirectoryContents(const std::string &path);
std::string buildAutoIndexHTML(const std::string &path, const std::string &requestPath);