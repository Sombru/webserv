#include "HTTP.hpp"

void HTTP::buildErrorRespose(int code)
{
	response.status_code = code;
	response.status_text = getStatusText(code);
	response.body = loadErrorPage(code);
	response.headers["Content-Type"] = getMimeType(serverConfig.root + "/" + serverConfig.errorPage);
	response.headers["Content-Length"] = intToString(response.body.size());
}


void HTTP::buildResponse(int code, std::string &fsTarget)
{
	response.status_code = code;
	response.status_text = getStatusText(code);
	std::string buffer = readFile(fsTarget);
	if (buffer == BADFILE)
		return buildErrorRespose(404);
	response.body = buffer;
	response.body = replacePlaceHolders(buffer, "{{file_list}}", generateFileListHtml((request.best_location.fs_uploadDir)));
	response.headers["Content-Type"] = getMimeType(fsTarget);
	response.headers["Content-Length"] = intToString(response.body.size());
}

void HTTP::buildResponse(int code)
{
	response.status_code = code;
	response.status_text = getStatusText(code);
}

std::string HTTP::generateFileListHtml(const std::string &directory)
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

void HTTP::redirect(const std::string &returnPath)
{
	buildResponse(302);
	addHeaders("Location", returnPath);
}

