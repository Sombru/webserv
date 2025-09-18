#include "HTTP.hpp"

std::string HTTP::resolveRequestPath()
{
	if (!request.best_location)
	{
		if (!request.path.c_str() || request.path == "/")
			request.path += "/" + serverConfig.index;
		return serverConfig.root + request.path;
	}
	
	if (request.path == request.best_location->path)
		request.path += "/" + request.best_location->index;
	DEBUG(request.best_location->fs_path);
	request.path.erase(0, request.best_location->path.size());


	return request.best_location->fs_path + request.path;
}

void HTTP::GET()
{
	std::string fsPath = resolveRequestPath();

	DEBUG("Resolved path for a request: " + fsPath);
	// Check if file exists
	std::ifstream testFile(fsPath.c_str());
	if (!testFile.good())
	{
		// File not found - serve error page
		response.status_code = 404;
		response.status_text = "Not Found";
		response.body = loadErrorPage(404, "Not Found");
		response.headers["Content-Type"] = "text/html";
		response.headers["Content-Length"] = intToString(response.body.size());
		return;
	}

	// Determine content type
	std::string mime = getMimeType(fsPath);

	// DEBUG(mime);
	// Read file content
	std::string bodyContent = readFile(fsPath);

	// Handle case where file exists but can't be read
	if (bodyContent == BADFILE)
	{
		response.status_code = 500;
		response.status_text = "Internal Server Error";
		response.body = loadErrorPage(500, "Internal Server Error");
		response.headers["Content-Type"] = "text/html";
		response.headers["Content-Length"] = intToString(response.body.size());
		return;
	}

	// Set successful response
	response.status_code = 200;
	response.status_text = "OK";
	response.body = bodyContent;
	response.headers["Content-Type"] = mime;
	// response.headers["Content-Type"] = "text/html";
	response.headers["Content-Length"] = intToString(response.body.size());

	// Ensure proper content disposition
	if (mime.find("text/") == 0 || mime == "application/javascript")
	{
		response.headers["Content-Disposition"] = "inline";
	}
}