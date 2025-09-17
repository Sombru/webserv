#include "HTTP.hpp"

void HTTP::GET()
{
	std::string requestedPath = request.path;
	if (requestedPath.empty() || requestedPath == "/")
		requestedPath = "/index.html";

	// Build filesystem path from server root
	std::string fsPath = serverConfig.root + requestedPath;

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

	// Read file content
	std::string bodyContent = readFileBinary(fsPath);

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
	response.headers["Content-Length"] = intToString(response.body.size());

	// Ensure proper content disposition
	if (mime.find("text/") == 0 || mime == "application/javascript")
	{
		response.headers["Content-Disposition"] = "inline";
	}
}