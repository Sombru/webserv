#include "HTTP.hpp"
#include <dirent.h>

std::string generateFileListHtml(const std::string &directory)
{
	std::string html;
	DIR *dir = opendir(directory.c_str());
	if (!dir)
	{
		DEBUG("Failed to open directory: " + directory +
			  " - Error: " + strerror(errno));
		return "<li>No files available</li>";
	}

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string filename = entry->d_name;
		// Skip hidden files and directories
		if (filename[0] == '.' || filename == ".." || filename == ".")
		{
			continue;
		}

		// Create list items that match your existing HTML structure
		html += "<li id=\"file-" + filename + "\">\n";
		html += "  <a href=\"/upload/" + filename + "\" download>" + filename +
				"</a>\n";
		html += "  <button class=\"delete-btn\" onclick=\"deleteFile('" +
				filename + "')\">Delete</button>\n";
		html += "</li>\n";
	}

	if (html.empty())
	{
		html = "<li>No files uploaded yet</li>";
	}

	closedir(dir);
	return html;
}

std::string HTTP::resolveRequestPath()
{
	if (!request.best_location)
	{
		if (!request.path.c_str() || request.path == "/")
			request.path += serverConfig.index;
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
	if (request.path == "/" || request.path == "/index.html")
	{
		// Check if user is logged in via cookie
		if (request.cookies.find("logged_in") == request.cookies.end() ||
			request.cookies["logged_in"] != "true")
		{
			// Not logged in, redirect to login
			response.status_code = 302;
			response.status_text = "Found";
			response.headers["Location"] = "/login.html";
			response.body = "";
			return;
		}
	}

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

	// Check if this is index.html and replace {{file_list}} placeholder
	if (fsPath.find("index.html") != std::string::npos)
	{
		std::string fileListHtml = generateFileListHtml("./upload");
		size_t pos = bodyContent.find("{{file_list}}");
		if (pos != std::string::npos)
		{
			bodyContent.replace(pos, 14,
								fileListHtml); // 14 = length of "{{file_list}}"
		}
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