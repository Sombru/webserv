#include "Webserv.hpp"
#include "HTTP.hpp"
#include "Config.hpp"
#include "Logger.hpp"

std::string loadTemplateErrorPage(int code, const std::string& message);
std::string getStatusText(int code);

HttpResponse generateErrorResponse(int code, const std::string &error_pages_dir, const std::string& version)
{
	HttpResponse resp;
	resp.status_code = code;
	resp.status_text = getStatusText(resp.status_code);
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
	// // 	//Default error page
	// 	resp.body = "<html><head><title>" + intToString(code) + " " + resp.status_text + "</title></head>"
	// 																						"<body><h1>" +
	// 				intToString(code) + " " + resp.status_text + "</h1></body></html>";
	// }
	resp.body = loadTemplateErrorPage(code, resp.status_text);
	}
	resp.headers["Content-Type"] = "text/html";
	resp.headers["Content-Length"] = intToString(resp.body.size());
	std::cout << "\n\033[31mtext: " << resp.status_text << "\033[0m";
	std::cout << "\n\033[33mcode: " << resp.status_code << "\033[0m";
	std::cout << "\n\033[32mbody: " << resp.body << "\033[0m";
	std::cout << std::endl;
	return resp;
}

std::string loadTemplateErrorPage(int code, const std::string& message) {
	std::ifstream file("www/errors/error_template.html");
	if (!file.is_open()) {
		Logger::error("Failed to open error_template.html");
		return "<h1>" + intToString(code) + " " + message + "</h1>";
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string content = buffer.str();

	// Replace placeholders
	size_t pos;
	while ((pos = content.find("{{code}}")) != std::string::npos)
		content.replace(pos, 8, intToString(code));
	while ((pos = content.find("{{message}}")) != std::string::npos)
		content.replace(pos, 11, message);

	return content;
}


std::string getStatusText(int code) {
	switch (code) {
		case 200: return "Ok";
		case 400: return "Bad Request";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method not allowed";
		case 413: return "Payload Too Large";
		case 500: return "Internal Server Error";
		case 502: return "Bad Gateway";
		default: return "Error";
	}
}