#include "HTTP.hpp"
#include "Webserv.hpp"
#include "Config.hpp"
#include "Logger.hpp"

HttpResponse GET(const HttpRequest &request, const ServerConfig &server)
{
	std::string fs_path = server.root;
	fs_path += request.best_location->root + request.target_file;

	std::string body = readFile(fs_path);
	std::vector<std::string> contents = getLocationContents(fs_path);

	Logger::debug(fs_path);
	// Logger::debug(contents);

	if (body != "BAD")
	{
		Logger::debug(body);
		return buildResponse(OK, body, HTTPVERSION);
	}
	else if (!contents.empty())
	{
		Logger::debug(fs_path += request.best_location->index);
		body = readFile(fs_path + request.best_location->index);
		return buildResponse(OK, body, HTTPVERSION);
	}
	return buildErrorResponse(NOTFOUD, server.error_pages_dir, HTTPVERSION);

	// if (request.best_location->autoindex && fs_path.empty())
	// {
	// 	body = "<ul>";
	// 	for (size_t i = 0; i < contents.size(); ++i)
	// 		body += "<li>" + contents[i] + "</li>";
	// 	body += "</ul>";
	// 	return buildResponse(OK, body, HTTPVERSION);
	// }
	// else if (!request.best_location->index.empty() && fs_path.empty())
	// {

	// }

}