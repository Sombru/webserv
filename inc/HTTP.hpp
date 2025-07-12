#pragma once

#include "Webserv.hpp"
#include "Config.hpp"

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
	std::string fullPath; // e.g. "/" or "/about.html"
	std::string version; // e.g. "HTTP/1.1"
	int status_code; // e.g. 200
	std::string status_text; // e.g. "OK"
	std::map<std::string, std::string> headers; // e.g. headers["Content-Length"] == body.size()
	std::string body; // e.g. Hello, world!
};

HttpRequest parseRequset(const std::string& raw_request, const ServerConfig& config);
HttpResponse recieveResponse(HttpRequest request, const ServerConfig& config);
std::string serialize(HttpResponse &response);
HttpResponse tooLargeRequest();
