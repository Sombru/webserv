#include "HTTP.hpp"
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

void HTTP::DELETE()
{
	std::string fsPath;

	// Special handling for upload location due to config issue
	if (request.best_location && request.best_location->path == "/upload")
	{
		// Extract filename after /upload/
		std::string filename = request.path;
		if (filename.find("/upload/") == 0)
		{
			filename = filename.substr(8); // Remove "/upload/" prefix
		}
		fsPath = "./upload/" + filename;
	}
	else
	{
		fsPath = resolveRequestPath();
	}

	DEBUG("DELETE request for path: " + fsPath);
	DEBUG("Original request path: " + request.path);
	DEBUG("Best location: " +
		  (request.best_location ? request.best_location->path : "NULL"));

	// Check if we're in an upload location that allows DELETE
	if (!request.best_location)
	{
		response.status_code = 403;
		response.status_text = "Forbidden";
		response.body = loadErrorPage(403, "Forbidden");
		response.headers["Content-Type"] = "text/html";
		response.headers["Content-Length"] = intToString(response.body.size());
		return;
	}

	// Check if DELETE method is allowed for this location
	bool deleteAllowed = false;
	for (size_t i = 0; i < request.best_location->allowedMethods.size(); ++i)
	{
		if (request.best_location->allowedMethods[i] == "DELETE")
		{
			deleteAllowed = true;
			break;
		}
	}

	if (!deleteAllowed)
	{
		response.status_code = 405;
		response.status_text = "Method Not Allowed";
		response.body = loadErrorPage(405, "Method Not Allowed");
		response.headers["Content-Type"] = "text/html";
		response.headers["Content-Length"] = intToString(response.body.size());
		response.headers["Allow"] = "GET, POST";
		return;
	}

	// Check if file exists
	struct stat fileStat;
	if (stat(fsPath.c_str(), &fileStat) != 0)
	{
		response.status_code = 404;
		response.status_text = "Not Found";
		response.body = loadErrorPage(404, "File Not Found");
		response.headers["Content-Type"] = "text/html";
		response.headers["Content-Length"] = intToString(response.body.size());
		return;
	}

	// Check if it's a regular file (not a directory)
	if (!S_ISREG(fileStat.st_mode))
	{
		response.status_code = 403;
		response.status_text = "Forbidden";
		response.body = loadErrorPage(403, "Cannot delete directories");
		response.headers["Content-Type"] = "text/html";
		response.headers["Content-Length"] = intToString(response.body.size());
		return;
	}

	// Attempt to delete the file
	if (remove(fsPath.c_str()) == 0)
	{
		response.status_code = 200;
		response.status_text = "OK";
		response.body = "";
		response.headers["Content-Length"] = "0";
		DEBUG("Successfully deleted file: " + fsPath);
	}
	else
	{
		// Failed to delete
		response.status_code = 500;
		response.status_text = "Internal Server Error";
		response.body = loadErrorPage(500, "Failed to delete file: " +
											   std::string(strerror(errno)));
		response.headers["Content-Type"] = "text/html";
		response.headers["Content-Length"] = intToString(response.body.size());
		DEBUG("Failed to delete file: " + fsPath +
			  " - Error: " + strerror(errno));
	}
}