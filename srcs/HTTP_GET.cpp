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

void HTTP::GET(std::string &fsPath)
{


	if (is_directory(fsPath))
	{
		buildResponse(200, request.best_location.fs_index);
		return ;
	}

	return buildResponse(200, fsPath);


	// Check if this is index.html and replace {{file_list}} placeholder
	// if (fsPath.find("index.html") != std::string::npos)
	// {
	// 	std::string fileListHtml = generateFileListHtml("./upload");
	// 	size_t pos = bodyContent.find("{{file_list}}");
	// 	if (pos != std::string::npos)
	// 	{
	// 		bodyContent.replace(pos, 14,
	// 							fileListHtml); // 14 = length of "{{file_list}}"
	// 	}
	// }

	// Set successful response

	// Ensure proper content disposition
	// if (mime.find("text/") == 0 || mime == "application/javascript")
	// {
	// 	response.headers["Content-Disposition"] = "inline";
	// }
}
