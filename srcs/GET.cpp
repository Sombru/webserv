#include "HTTP.hpp"
#include "Webserv.hpp"
#include "Config.hpp"
#include "Logger.hpp"

HttpResponse GET(const HttpRequest &request, const ServerConfig &server)
{
	std::string fs_path = server.root;
	fs_path += request.best_location->root;

	std::vector<std::string> contents = getLocationContents(fs_path);
	std::string body;

	// Logger::debug(fs_path);
	// Logger::debug(contents);
	// body = readFile(fs_path + request.best_location->index);

	if (!request.target_file.empty())
	{
		fs_path += request.target_file;
		body = readFile(fs_path);
		if (body == "BAD")
			return buildErrorResponse(NOTFOUD, server.error_pages_dir, HTTPVERSION);
		Logger::debug(body);
		return buildResponse(OK, body, HTTPVERSION);
	}
	else if (!contents.empty())
	{
		if (request.best_location->index.empty())
			return buildErrorResponse(FORBIDEN, server.error_pages_dir, HTTPVERSION);
		body = readFile(fs_path + request.best_location->index);
		if (request.best_location->autoindex)
		{
			body += "<ul>";
			for (size_t i = 0; i < contents.size(); ++i)
				body += "<li>" + contents[i] + "</li>";
			body += "</ul>";
			return buildResponse(OK, body, HTTPVERSION);
		}
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