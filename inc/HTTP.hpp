#pragma once

#include "Webserv.hpp"
#include "Config.hpp"

struct HttpRequest
{
	std::string method; // e.g. GET
	const LocationConfig* best_location; // e.g. /files
	std::string path; // e.g. /files/about.html
	std::string fs_path; // e.g. www/./files/about.html (this is local file system path)
	std::string version; //  e.g. "HTTP/1.1"
	std::map<std::string, std::string> query_params; // e.g. query_params["alice"] == 18
	std::map<std::string, std::string> headers; // e.g. headers["Authorization"] == <browser>
	std::string body;
};

HttpRequest parseRequest(const std::string &rawRequest, const ServerConfig &server);
const LocationConfig* findBestLocation(const std::string &path, const ServerConfig &server);

#define SUCCESS 200
#define OK 200
#define CREATED 201
#define ACCEPTED 202
#define NO_CONTENT 204

#define MOVED_PERMANENTLY 301
#define FOUND 302
#define SEE_OTHER 303
#define NOT_MODIFIED 304
#define TEMPORARY_REDIRECT 307
#define PERMANENT_REDIRECT 308

#define BAD_REQUEST 400
#define UNAUTHORIZED 401
#define FORBIDDEN 403
#define NOTFOUND 404
#define METHOD_NOT_ALLOWED 405
#define NOT_ACCEPTABLE 406
#define REQUEST_TIMEOUT 408
#define CONFLICT 409
#define GONE 410
#define LENGTH_REQUIRED 411
#define PAYLOAD_TOO_LARGE 413
#define URI_TOO_LONG 414
#define UNSUPPORTED_MEDIA_TYPE 415

#define INTERNAL_SERVER_ERROR 500
#define NOT_IMPLEMENTED 501
#define BAD_GATEWAY 502
#define SERVICE_UNAVAILABLE 503
#define GATEWAY_TIMEOUT 504

#define HTTPVERSION "HTTP/1.1"

struct HttpResponse 
{
	int status_code; // e.g. 200
	std::string status_text; // e.g. "OK"
	std::string version; // e.g. "HTTP/1.1"
	std::map<std::string, std::string> headers; // e.g. headers["Content-Length"] == body.size()
	std::string body; // e.g. Hello, world!
};

HttpResponse generateResponse(const HttpRequest &request, const ServerConfig &serverConfig);

HttpResponse GET(const HttpRequest &request, const ServerConfig &server);

// builders

HttpResponse buildErrorResponse(int code, const ServerConfig& server);
HttpResponse buildRedirction(const HttpRequest &request);
HttpResponse buildSuccessResponse(int code, const std::string &body);
inline HttpResponse buildResponse(int code, const std::string &body, const ServerConfig& server);
