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

HttpResponse buildSuccessResponse(int code, const std::string &body);

HttpResponse buildErrorResponse(int code, const ServerConfig& server);

inline HttpResponse buildResponse(int code, const std::string &body, const ServerConfig& server);

HttpResponse GET(const HttpRequest& request, const ServerConfig& server);

std::string loadErrorPage(int code, const ServerConfig& server);

