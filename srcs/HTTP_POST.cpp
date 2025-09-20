#include "HTTP.hpp"
#include <fstream>
#include <sys/stat.h>

void HTTP::POST()
{
	// Check if we have an upload directory configured
	if (!request.best_location || request.best_location->uploadDir.empty())
	{
		response.status_code = 403;
		response.status_text = "Forbidden";
		response.body = loadErrorPage(403, "Forbidden");
		response.headers["Content-Type"] = "text/html";
		response.headers["Content-Length"] = intToString(response.body.size());
		return;
	}

	std::string uploadDir = request.best_location->uploadDir;

	// Create upload directory if it doesn't exist
	struct stat st;
	if (stat(uploadDir.c_str(), &st) == -1)
	{
		if (mkdir(uploadDir.c_str(), 0755) == -1)
		{
			response.status_code = 500;
			response.status_text = "Internal Server Error";
			response.body = loadErrorPage(500, "Internal Server Error");
			response.headers["Content-Type"] = "text/html";
			response.headers["Content-Length"] =
				intToString(response.body.size());
			return;
		}
	}

	// Extract filename from Content-Disposition header if present
	std::string filename;
	if (request.headers.find("Content-Disposition") != request.headers.end())
	{
		std::string disposition = request.headers["Content-Disposition"];
		size_t filenamePos = disposition.find("filename=");
		if (filenamePos != std::string::npos)
		{
			filename = disposition.substr(filenamePos + 9);
			if (filename[0] == '"' && filename[filename.size() - 1] == '"')
			{
				filename = filename.substr(1, filename.size() - 2);
			}
		}
	}

	// If no filename in header, generate one
	if (filename.empty())
	{
		filename = "upload_" + getTimestamp() + ".dat";
		// Replace characters that might be problematic in filenames
		for (size_t i = 0; i < filename.size(); i++)
		{
			if (filename[i] == ' ' || filename[i] == ':')
				filename[i] = '_';
		}
	}

	std::string fullPath = uploadDir + "/" + filename;

	// Write the file
	std::ofstream outFile(fullPath.c_str(), std::ios::binary);
	if (!outFile)
	{
		response.status_code = 500;
		response.status_text = "Internal Server Error";
		response.body = loadErrorPage(500, "Internal Server Error");
		response.headers["Content-Type"] = "text/html";
		response.headers["Content-Length"] = intToString(response.body.size());
		return;
	}

	outFile.write(request.body.c_str(), request.body.size());
	outFile.close();

	// Load success.html template
	std::string successTemplate =
		readFile(serverConfig.root + "/success/success.html");
	if (successTemplate == BADFILE)
	{
		// Fallback to simple message if template not found
		response.status_code = 201;
		response.status_text = "Created";
		response.body =
			"<html><body><h1>File uploaded successfully</h1><p>Filename: " +
			filename + "</p></body></html>";
	}
	else
	{
		// Replace placeholder with actual file path
		size_t pos = 0;
		while ((pos = successTemplate.find("{{filepath}}", pos)) !=
			   std::string::npos)
		{
			successTemplate.replace(pos, 12, fullPath);
			pos += fullPath.length();
		}

		response.status_code = 201;
		response.status_text = "Created";
		response.body = successTemplate;
	}

	response.headers["Content-Type"] = "text/html";
	response.headers["Content-Length"] = intToString(response.body.size());
	response.headers["Location"] = "/upload/" + filename;
}