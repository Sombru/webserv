#include "Webserv.hpp"
#include "HTTP.hpp"
#include "Config.hpp"

void generateHeaders(HttpResponse& response)
{
	response.headers["Content-Type"] = "text/html";
	response.headers["Content-Length"] = intToString(response.body.size());
	response.headers["Connection"] = "close";
	response.headers["Server"] = "skbidi rizzlers";
}

HttpResponse generateErrorResponse(int code, const std::string &error_pages_dir, const std::string& version)
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