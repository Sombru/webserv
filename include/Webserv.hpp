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

// preprocessor for hash map
#define strHmap std::map<std::string, std::string>
#define INDEX "webpages/index.html"

struct HttpRequest
{
	std::string method;
	std::string path;
	std::string query_string;
	std::string version;
	strHmap query_params;
	strHmap headers;
};

struct HttpResponse 
{
	std::string version;        // e.g. "HTTP/1.1"
	int status_code;            // e.g. 200
	std::string status_text;    // e.g. "OK"
	strHmap headers;
	std::string body;
};

// TODO add a httpresponse structure

void openFile(std::ifstream& file, const std::string& filename);

strHmap parse_query(const std::string &query_string);

HttpRequest parseRequest(const std::string &raw_request);

std::ostream &operator<<(std::ostream &os, const HttpRequest &req);


#endif