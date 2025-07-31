#pragma once

#include "Webserv.hpp"
#include "Config.hpp"

struct HttpRequest
{
	std::string method; // e.g. GET
	const LocationConfig* best_location; // e.g. /files
	std::string path; // e.g. /files/about.html (this is local file system path)
	std::string version; //  e.g. "HTTP/1.1"
	std::map<std::string, std::string> query_params; // e.g. query_params["alice"] == 18
	std::map<std::string, std::string> headers; // e.g. headers["Authorization"] == <browser>
	std::string body;
};

HttpRequest parseRequest(const std::string &rawRequest, const ServerConfig &server);
const LocationConfig* findBestLocation(const std::string &path, const ServerConfig &server);

struct HttpResponse 
{
	int status_code; // e.g. 200
	std::string status_text; // e.g. "OK"
	std::string version; // e.g. "HTTP/1.1"
	std::map<std::string, std::string> headers; // e.g. headers["Content-Length"] == body.size()
	std::string body; // e.g. Hello, world!
};

