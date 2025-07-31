#include "Webserv.hpp"
#include "Utils.hpp"
#include "HTTP.hpp"
#include "Logger.hpp"

// takes a path to a file and returns it contents
/// @returns file contens, BADFILE if cant open/read, EPMTY if contents is empty
std::string readFile(const std::string &path)
{
	std::ifstream file(path.c_str());
	if (!file)
		return (BADFILE);
	std::stringstream buffer;

	buffer << file.rdbuf();

	if (buffer.str().empty())
		return (EMPTY);
	return (buffer.str());
}

std::string intToString(int n)
{
	std::stringstream ss;
	ss << n;
	return ss.str();
}

std::string serialize(const HttpResponse &response)
{
	std::ostringstream oss;

	oss << response.version << " " << response.status_code << " " << response.status_text << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it = response.headers.begin(); it != response.headers.end(); ++it)
	{
		oss << it->first << ": " << it->second << "\r\n";
	}
	oss << "\r\n";
	oss << response.body;

	return oss.str();
}

// helper to check if "path" is a directory 
bool is_directory(const std::string &path)
{
	struct stat info;
	if (stat(path.c_str(), &info) != 0)
	{
		return false; // cannot access path
	}
	return (info.st_mode & S_IFDIR) != 0;
}

// gets directory contents in a specified path
std::vector<std::string> getDirectoryContents(const std::string &path)
{
	std::vector<std::string> contents;

	DIR *dir = opendir(path.c_str());
	if (!dir)
		return contents; // Could not open directory

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;
		if (name != "." && name != "..")
			contents.push_back(name);
	}
	closedir(dir);
	return contents;
}

// builds HTML page for directory listing (autoindex)
std::string buildAutoIndexHTML(const std::string &path, const std::string &requestPath)
{
	std::string html;
	std::vector<std::string> contents = getDirectoryContents(path);

	// HTML header
	html += "<!DOCTYPE html>\n";
	html += "<html>\n<head>\n";
	html += "<title>Index of " + requestPath + "</title>\n";
	html += "<style>\n";
	html += "body { font-family: Arial, sans-serif; margin: 40px; }\n";
	html += "h1 { color: #333; }\n";
	html += "ul { list-style-type: none; padding: 0; }\n";
	html += "li { margin: 5px 0; }\n";
	html += "a { text-decoration: none; color: #0066cc; }\n";
	html += "a:hover { text-decoration: underline; }\n";
	html += ".directory::before { content: '📁 '; }\n";
	html += ".file::before { content: '📄 '; }\n";
	html += "</style>\n";
	html += "</head>\n<body>\n";
	html += "<h1>Index of " + requestPath + "</h1>\n";
	html += "<ul>\n";

	// Add parent directory link if not root
	if (requestPath != "/")
	{
		html += "<li><a href=\"../\" class=\"directory\">../</a></li>\n";
	}

	// Add directory entries
	for (size_t i = 0; i < contents.size(); ++i)
	{
		std::string entry = contents[i];
		std::string fullPath = path + "/" + entry;
		
		// Check if entry is a directory
		if (is_directory(fullPath))
		{
			html += "<li><a href=\"" + entry + "/\" class=\"directory\">" + entry + "/</a></li>\n";
		}
		else
		{
			html += "<li><a href=\"" + entry + "\" class=\"file\">" + entry + "</a></li>\n";
		}
	}

	html += "</ul>\n</body>\n</html>";
	return html;
}

// // gets directory contets in a specified location of a server
// std::vector<std::string> getLocationContents(const std::string &server_root, const std::string &location_root)
// {
// 	std::vector<std::string> contents;
// 	std::string full_path = server_root + location_root;

// 	DIR *dir = opendir(full_path.c_str());
// 	if (!dir)
// 		return contents; // Could not open directory

// 	struct dirent *entry;
// 	contents.reserve(10); // buffer
// 	while ((entry = readdir(dir)) != NULL)
// 	{
// 		std::string name = entry->d_name;
// 		if (name != "." && name != "..")
// 			contents.push_back(name);
// 	}
// 	closedir(dir);
// 	return contents;
// }

// // gets directory contets in a specified location of a server
// std::vector<std::string> getLocationContents(const std::string& path)
// {
// 	std::vector<std::string> contents;

// 	DIR *dir = opendir(path.c_str());
// 	if (!dir)
// 		return contents; // Could not open directory

// 	struct dirent *entry;
// 	contents.reserve(10); // buffer
// 	while ((entry = readdir(dir)) != NULL)
// 	{
// 		std::string name = entry->d_name;
// 		if (name != "." && name != "..")
// 			contents.push_back(name);
// 	}
// 	closedir(dir);
// 	return contents;
// }

// std::string getFileExtension(const std::string& file)
// {
// 	size_t pos = file.find_last_of('.');
// 	if (pos == std::string::npos || pos == file.length() - 1)
// 		return "";
// 	return file.substr(pos);
// }

// bool isCGIextension(const std::string& ext, const LocationConfig* loc)
// {
// 	for (size_t i = 0; i < loc->cgi_ext.size(); i++)
// 		if (loc->cgi_ext[i] == ext)
// 			return true;
// 	return false;
// }

// std::string buildAutoIndex(const std::string& fs_path)
// {
// 	std::string body;
// 	std::vector<std::string> contents = getLocationContents(fs_path);

// 	// Add parent directory link if not root
// 	if (fs_path != "/")
// 		body += "<li><a href=\"../\">../</a></li>\n";
// 	for (size_t i = 0; i < contents.size(); ++i)
// 	{
// 		std::string entry = contents[i];
// 		body += "<li><a href=\"" + entry + "\">" + entry + "</a></li>\n";
// 	}
// 	body += "</ul>\n</body>\n</html>";
// 	return body;
// }