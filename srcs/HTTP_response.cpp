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

std::string loadTemplateErrorPage(int code, const std::string &status_text)
{
	std::string content = readFile(DEFAULTERRORPAGE);
	if (content == "BAD")
		return "<h1>" + intToString(code) + " " + status_text + "</h1>";

	// Replace placeholders
	size_t pos;
	while ((pos = content.find("{{code}}")) != std::string::npos)
		content.replace(pos, 8, intToString(code));
	while ((pos = content.find("{{status_text}}")) != std::string::npos)
		content.replace(pos, 15, status_text);

	return content;
}

HttpResponse buildErrorResponse(int code, const std::string &error_pages_dir, const std::string &version)
{
	HttpResponse resp;
	resp.status_code = code;
	resp.status_text = getStatusText(code);
	resp.version = version;

	// Try to read custom error page
	std::string filePath = error_pages_dir + "/" + intToString(code) + ".html";
	// std::string body = readFile(filePath);
	// if (body != "BAD")
	// 	resp.body = body;

	// else

	// Default error page
	resp.body = loadTemplateErrorPage(code, resp.status_text);

	resp.headers = generateHeaders(resp);
	return resp;
}

HttpResponse buildResponse(int code, const std::string &body, const std::string &version)
{
	HttpResponse response;
	response.status_code = code;
	response.status_text = getStatusText(code);
	response.version = version;
	response.body = body;
	response.headers = generateHeaders(response);
	return response;
}