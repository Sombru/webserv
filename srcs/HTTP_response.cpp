#include "Webserv.hpp"
#include "HTTP.hpp"
#include "Config.hpp"

std::map<std::string, std::string> generateHeaders(const HttpResponse& response)
{
	std::map<std::string, std::string> headers;

	headers["Content-Type"] = "text/html";
	headers["Content-Length"] = intToString(response.body.size());
	headers["Connection"] = "close";
	headers["Server"] = "skbidi_rizzlers";
	return headers;
}

std::string getStatusText(int code)
{
	switch (code)
	{
	case 200:
		return "Ok";
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

HttpResponse buildErrorResponse(int code, const std::string& error_pages_dir, const std::string &version)
{
	HttpResponse resp;
	resp.status_code = code;
	resp.status_text = "error";
	resp.version = version;
	resp.headers.clear();

	// Try to read custom error page
	std::string filePath = error_pages_dir + "/" + intToString(code) + ".html";
	std::string body = readFile(filePath);
	if (body != "BAD")
		resp.body = body;
	else
	{
		// Default error page
		resp.body = "<html><head><title>" + intToString(code) + " " + resp.status_text + "</title></head>"
																						 "<body><h1>" +
					intToString(code) + " " + resp.status_text + "</h1></body></html>";
	}
	generateHeaders(resp);
	return resp;
}

HttpResponse buildResponse(int code, const std::string& body, const std::string &version)
{
	HttpResponse response;
	response.status_code = code;
	response.status_text = getStatusText(code);
	response.version = version;
	response.body = body;
	response.headers = generateHeaders(response);
	return response;
}