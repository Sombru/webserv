#include "HTTP.hpp"
#include "Webserv.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include <sys/stat.h>
#include "Utils.hpp"

HttpResponse DELETE(HttpRequest request, const ServerConfig &server)
{
	HttpResponse response;

	// get file path from request
	Logger::debug(request.fs_path);
	std::string full_path = request.fs_path;

	// check if file exists
	struct stat fileStat;
	if (stat(full_path.c_str(), &fileStat) != 0)
	{
		return buildErrorResponse(404, server); // File not found
	}

	// handling check if in case someone would try to delete directory instead of a file
	if (!S_ISREG(fileStat.st_mode))
	{
		return buildErrorResponse(403, server); // Forbidden (trying to delete directory not a file)
	}

	// Try to delelte
	if (remove(full_path.c_str()) != 0)
	{
		return buildErrorResponse(500, server); // Failed to delete
	}

	response.status_code = 200;
	response.status_text = "OK";
	response.body = "File deleted: " + request.path;
	response.headers["Content-Length"] = intToString(response.body.size());
	response.headers["Content-Type"] = "text/plain";
	response.version = HTTPVERSION;

	return response;
}