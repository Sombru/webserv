#include "Webserv.hpp"
#include "HTTP.hpp"
#include "Config.hpp"
#include "Logger.hpp"

#define DEFAULTERRORPAGE "www/errors/default.html"

std::map<std::string, std::string> generateHeaders(const HttpResponse &response)
{
	std::map<std::string, std::string> headers;

	headers["Content-Type"] = "text/html";
	headers["Content-Length"] = intToString(response.body.length());
	headers["Connection"] = "close";
	headers["Server"] = "skbidi_rizzlers";
	return headers;
}

std::string getStatusText(int code)
{
	switch (code)
	{
	case 200:
		return "OK";
	case 201:
		return "Created";
	case 400:
		return "Bad Request";
	case 403:
		return "Forbidden";
	case 404:
		return "Not Found";
	case 405:
		return "Method not allowed";
	case 413:
		return "Payload Too Large";
	case 500:
		return "Internal Server Error";
	case 502:
		return "Bad Gateway";
	default:
		return "Error";
	}
}

std::string loadErrorPage(int code, const ServerConfig& server)
{
	std::string status_text = getStatusText(code);
	std::string body = readFile(server.error_pages_dir + "/" + intToString(code) + ".html");
	if (body != "BAD")
		return body;
	body = readFile(DEFAULTERRORPAGE);
	if (body == "BAD")
		return "<h1>" + intToString(code) + " " + status_text + "</h1>";
	
	// Replace placeholders
	size_t pos;
	while ((pos = body.find("{{code}}")) != std::string::npos)
		body.replace(pos, 8, intToString(code));
	while ((pos = body.find("{{status_text}}")) != std::string::npos)
		body.replace(pos, 15, status_text);

	return body;
}

// builds a HTTP response with given error and Server error page
HttpResponse buildErrorResponse(int code, const ServerConfig& server)
{
	HttpResponse response;
	response.status_code = code;
	response.status_text = getStatusText(code);
	response.version = HTTPVERSION;
	response.body = loadErrorPage(code, server);
	response.headers = generateHeaders(response);
	return response;
}

// builds a HTTP response with given succes code and body,
// use overload for error responses
HttpResponse buildSuccessResponse(int code, const std::string &body)
{
	HttpResponse response;
	response.status_code = code;
	response.status_text = getStatusText(code);
	response.version = HTTPVERSION;
	response.body = body;
	response.headers = generateHeaders(response);
	return response;
}

// builds a HTTP response with given code, body and server
inline HttpResponse buildResponse(int code, const std::string &body, const ServerConfig& server)
{
	if (code >= 400)
		return buildErrorResponse(code, server);
	if (code < 400)
		return buildSuccessResponse(code, body);
	return(buildErrorResponse(500, server));
}