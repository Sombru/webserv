#include "Webserv.hpp"
#include "HTTP.hpp"
#include "Config.hpp"

HttpResponse generateErrorResponse(int code, const std::string &error_pages_dir, const std::string& version)
{
	HttpResponse resp;
	resp.status_code = code;
	resp.status_text = "error";
	resp.version = version;
	resp.headers.clear();

	// Try to read custom error page
	std::string filePath = error_pages_dir + "/" + intToString(code) + ".html";
	std::ifstream file(filePath.c_str());
	if (file)
	{
		std::stringstream buffer;
		buffer << file.rdbuf();
		resp.body = buffer.str();
	}
	else
	{
		// Default error page
		resp.body = "<html><head><title>" + intToString(code) + " " + resp.status_text + "</title></head>"
																							"<body><h1>" +
					intToString(code) + " " + resp.status_text + "</h1></body></html>";
	}
	resp.headers["Content-Type"] = "text/html";
	resp.headers["Content-Length"] = intToString(resp.body.size());
	return resp;
}