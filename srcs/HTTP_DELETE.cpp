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
		buildErrorRespose(403);
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
		buildErrorRespose(405);
		return;
	}

	// Check if file exists
	struct stat fileStat;
	if (stat(fsPath.c_str(), &fileStat) != 0)
	{
		buildErrorRespose(404);
		return;
	}

	// Check if it's a regular file (not a directory)
	if (!S_ISREG(fileStat.st_mode))
	{
		buildErrorRespose(404);
		return;
	}

	// Attempt to delete the file
	if (remove(fsPath.c_str()) == 0)
	{
		buildResponse(200);
	}
	else
	{
		// Failed to delete
		buildErrorRespose(500);
	}
}