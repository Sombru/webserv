#include "Config.hpp"
#include "HTTP.hpp"
#include "Logger.hpp"
#include "Utils.hpp"

// HttpResponse handleRedirection(const HttpRequest &request, const ServerConfig &server)
// {
// 	for (size_t i = 0; i < server.locations.size(); i++)
// 	{
// 		if (request.best_location->returnPath == server.locations[i].name)
// 		{
// 			return ()
// 		}
// 	}

// }

HttpResponse GET(const HttpRequest &request, const ServerConfig &server)
{
	std::string body;
	// if (cgiExt(request.fs_path) == true)
	// 		return runCGI();
	// Logger::debug(request.fs_path + (is_directory(request.fs_path) ? " true": " false"));
	if (request.best_location->returnPath.empty() == false)
	{
		HttpResponse response;
		response.body = "Redirecting...";
		response.status_code = MOVED_PERMANENTLY;
		response.status_text = "Moved Permanently";
		response.version = "HTTP/1.1";
		response.headers["Location"] = request.best_location->returnPath;
		response.headers["Connection"] = "close";
		response.headers["Content-Length"] = intToString(response.body.size());
		return response;
	}
	if (is_directory(request.fs_path))
	{
		if (request.best_location->autoindex)
		{
			body = buildAutoIndexHTML(request.fs_path, request.path);
			return buildSuccessResponse(SUCCESS, body);
		}
		if (request.best_location->index.empty() == false)
		{
			body = readFile(request.fs_path + "/" + request.best_location->index);
			if (body == BADFILE)
				return buildErrorResponse(INTERNAL_SERVER_ERROR, server);
			return buildSuccessResponse(SUCCESS, body);
		}
		return buildErrorResponse(FORBIDDEN, server);
	}

	body = readFile(request.fs_path);
	if (body != BADFILE)
		return buildSuccessResponse(OK, body);

	return buildErrorResponse(NOTFOUND, server);
}