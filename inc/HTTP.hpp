#pragma once

#include "Webserv.hpp"
#include "Config.hpp"

struct HttpRequest
{
	std::string method; // e.g. GET
	std::string path; // e.g. /about.html
	std::string target_file; // e.g. about.html
	const LocationConfig* best_location; // e.g. /files/
	std::string query_string; // e.g ?alice=18 // how this differs from path you need find out
	std::string version; //  e.g. "HTTP/1.1"
	std::map<std::string, std::string> query_params; // e.g. query_params["alice"] == 18
	std::map<std::string, std::string> headers; // e.g. headers["Authorization"] == <browser>
	std::string body;
};

struct HttpResponse 
{
	int status_code; // e.g. 200
	std::string status_text; // e.g. "OK"
	std::string version; // e.g. "HTTP/1.1"
	std::map<std::string, std::string> headers; // e.g. headers["Content-Length"] == body.size()
	std::string body; // e.g. Hello, world!
};

#define OK 200

#define BADREQUEST 400
#define FORBIDEN 403
#define NOTFOUD 404

#define HTTPVERSION "HTTP/1.1"

HttpRequest parseRequset(const std::string& raw_request, const ServerConfig& config);
HttpResponse generateResponse(const HttpRequest&, const ServerConfig&);
HttpResponse buildErrorResponse(int code, const std::string& error_pages_dir, const std::string& version);
HttpResponse buildResponse(int code, const std::string& body, const std::string &version);

HttpResponse GET(const HttpRequest& request, const ServerConfig& server);

