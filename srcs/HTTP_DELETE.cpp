#include "HTTP.hpp"
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

void HTTP::DELETE(const std::string & fsPath)
{

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
		response.status_code = 200;
		response.status_text = "OK";
		response.body =	"<html><body><h1>File Deleted successfully</h1><p>Filename: " +
		fsPath.substr(fsPath.find_last_of('/')) + "</p></body></html>";
		response.headers["Content-Type"] = "text/html";
		response.headers["Content-Length"] = intToString(response.body.size());
		return ;
	}
	else
	{
		// Failed to delete
		buildErrorRespose(500);
	}
}