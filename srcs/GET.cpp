#include "HTTP.hpp"
#include "Webserv.hpp"
#include "Config.hpp"
#include "Logger.hpp"

HttpResponse run_cgi_script(const HttpRequest &request, const ServerConfig& server, const std::string& fs_path);

HttpResponse GET(const HttpRequest &request, const ServerConfig &server)
{
	std::string fs_path = server.root;
	fs_path += request.best_location->root;

	std::vector<std::string> contents = getLocationContents(fs_path);
	std::string body;

	if (request.best_location->name == "/cgi-bin/")
		return run_cgi_script(request, server, fs_path);

	if (!request.target_file.empty())
	{
		fs_path += request.target_file;
		body = readFile(fs_path);
		if (body == "BAD")
			return buildErrorResponse(NOTFOUD, server);

		// Inject file list if the target is the main page
		if (request.target_file == "index.html") {
			std::string fileListHtml = generateFileListHtml("uploads");
			size_t pos = body.find("{{file_list}}");
			if (pos != std::string::npos)
				body.replace(pos, 14, fileListHtml);  // 14 = length of "{{file_list}}"
		}

		return buildSuccessResponse(OK, body);
	}
	else if (!contents.empty())
	{
		if (request.best_location->index.empty())
			return buildErrorResponse(FORBIDEN, server);

		body = readFile(fs_path + request.best_location->index);

		// Same logic for index.html when not specified in URL
		if (request.best_location->index == "index.html") {
			std::string fileListHtml = generateFileListHtml("uploads");
			size_t pos = body.find("{{file_list}}");
			if (pos != std::string::npos)
				body.replace(pos, 14, fileListHtml);
		}

		if (request.best_location->autoindex)
		{
			body += "<ul>";
			for (size_t i = 0; i < contents.size(); ++i)
				body += "<li>" + contents[i] + "</li>";
			body += "</ul>";
			return buildSuccessResponse(OK, body);
		}
		return buildSuccessResponse(OK, body);
	}
	return buildErrorResponse(NOTFOUD, server);
}
