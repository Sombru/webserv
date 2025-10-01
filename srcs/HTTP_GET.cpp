#include "HTTP.hpp"
#include <dirent.h>

void HTTP::GET(std::string &fsPath)
{
	if (request.best_location.autoindex)
	{
		std::string html = buildAutoIndexHTML(fsPath);
		if (!request.best_location.fs_index.empty())
		{
			std::string buffer = readFile(request.best_location.fs_index);
			if (buffer == BADFILE)
				return buildErrorRespose(404);
			response.body = replacePlaceHolders(buffer, "{{autoindex}}", html);
		}
		else
			response.body = html;
		addHeaders("Content-Lenght", intToString(response.body.length()));
		return buildResponse(200);
	}
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

std::string HTTP::buildAutoIndexHTML(std::string &fsTarget)
{
	std::string html;
	std::vector<std::string> contents = getDirectoryContents(fsTarget);

	addHeaders("Content-Type", "text/html; charset=UTF-8");

	// HTML header
	html += "<html>\n<head>\n";
	html += "<style>\n";
	html += "body { font-family: Arial, sans-serif; margin: 40px; }\n";
	html += "h1 { color: #333; }\n";
	html += "ul { list-style-type: none; padding: 0; }\n";
	html += "li { margin: 8px 0; display: flex; align-items: center; }\n";
	html += "a { text-decoration: none; color: #0066cc; margin-right: 10px; }\n";
	html += "a:hover { text-decoration: underline; }\n";
	html += ".directory::before { content: '📁 '; }\n";
	html += ".file::before { content: '📄 '; }\n";
	html += "button { margin-left: auto; padding: 2px 8px; cursor: pointer; }\n";
	html += "</style>\n";

	// simple JS to handle delete
	html += "<script>\n";
	html += "function deleteFile(path) {\n";
	html += "  if (!confirm('Delete ' + path + '?')) return;\n";
	html += "  fetch(path, { method: 'DELETE' })\n";
	html += "    .then(res => {\n";
	html += "      if (res.ok) { location.reload(); }\n";
	html += "      else { alert('Failed to delete ' + path); }\n";
	html += "    });\n";
	html += "}\n";
	html += "</script>\n";

	html += "</head>\n<body>\n";
	html += "<h1>Index of " + request.path + "</h1>\n";
	html += "<ul>\n";

	// Add parent directory link if not root
	if (request.path != "/")
	{
		html += "<li><a href=\"" + request.path.substr(0, request.path.find_last_of('/')) 
		     + "\" class=\"directory\">../</a></li>\n";
	}

	if (contents.empty())
		html += "<li>No files available</li>";

	// Add directory entries
	for (size_t i = 0; i < contents.size(); ++i)
	{
		std::string entry = contents[i];
		std::string href;
		href = request.path + '/' + entry;

		std::string fullPath = fsTarget + "/" + entry;

		html += "<li>";
		if (is_directory(fullPath))
			html += "<a href=\"" + href + "\" class=\"directory\">" + entry + "/</a>";
		else
			html += "<a href=\"" + href + "\" class=\"file\">" + entry + "</a>";

		// Add delete button if DELETE is allowed
		for (size_t i = 0; i < request.best_location.allowedMethods.size(); ++i)
		{
			if (request.best_location.allowedMethods[i] == "DELETE" && !is_directory(fullPath))
				html += "<button onclick=\"deleteFile('" + href + "')\">Delete</button>";
		}
		
		html += "</li>\n";
	}

	html += "</ul>\n</body>\n</html>";
	return html;
}
