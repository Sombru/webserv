#include "HTTP.hpp"
#include <dirent.h>

void HTTP::GET(std::string &fsPath)
{
	if (request.best_location.autoindex && is_directory(fsPath))
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
		addHeaders("Content-Length", intToString(response.body.length()));
		return buildResponse(200);
	}
	if (is_directory(fsPath))
	{
		buildResponse(200, request.best_location.fs_index);
		return ;
	}

	// Check for CGI mapping by extension
	size_t dot = fsPath.find_last_of('.');
	std::string ext = (dot == std::string::npos) ? "" : fsPath.substr(dot + 1);
	// If location has cgi mapping for this extension or a 'none' mapping (executable), run CGI
	std::map<std::string, std::string>::iterator it = request.best_location.cgi.find(ext);
	if (it != request.best_location.cgi.end() || request.best_location.cgi.find("none") != request.best_location.cgi.end())
	{
		std::string interpreter;
		if (it != request.best_location.cgi.end()) 
			interpreter = it->second;
		else
			interpreter = request.best_location.cgi["none"];

		// Execute CGI and build response from its output
		if (executeCgi(fsPath, interpreter, ""))
			return; // response already filled by executeCgi
		else
			return; // executeCgi already set error response
	}

	return buildResponse(200, fsPath);
}

std::string HTTP::buildAutoIndexHTML(std::string &fsTarget)
{
	std::string html;
	std::vector<std::string> contents = getDirectoryContents(fsTarget);

	addHeaders("Content-Type", "text/html; charset=UTF-8");

	// HTML header
	html += "\n<head>\n";
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

	html += "</ul>\n</body>";
	return html;
}
