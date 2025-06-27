#ifndef LIBRARIES_HPP
# define LIBRARIES_HPP

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

// preprocessor for hash map
#define strHmap std::map<std::string, std::string>
#define INDEX "webpages/index.html"

struct HttpRequest
{
	std::string method; // e.g. GET
	std::string path; // e.g. /about.html
	std::string query_string; // e.g ?alice=18 // how this differs from path you need find out
	std::string version; //  e.g. "HTTP/1.1"
	std::map<std::string, std::string> query_params; // e.g. query_params["alice"] == 18
	std::map<std::string, std::string> headers; // e.g. headers["Authorization"] == <browser>
};

struct HttpResponse 
{
	std::string version; // e.g. "HTTP/1.1"
	int status_code; // e.g. 200
	std::string status_text; // e.g. "OK"
	std::map<std::string, std::string> headers; // e.g. headers["Content-Length"] == body.size()
	std::string body; // e.g. Hello, world!  
};


void openFile(std::ifstream& file, const std::string& filename);
strHmap parse_query(const std::string &query_string);
HttpRequest parseRequest(const std::string &raw_request);
std::ostream &operator<<(std::ostream &os, const HttpRequest &req);
void validateFormat(std::string filename);
void validateStartTitle(const std::string& line);
void validateTitles(std::ifstream &file);
void validateBraces(std::string filename);
void validateFormat(std::string filename);


#endif