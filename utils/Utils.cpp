#include "Webserv.hpp"
#include "Utils.hpp"
// #include "HTTP.hpp"

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

// std::string serialize(const HttpResponse &response)
// {
// 	std::ostringstream oss;

// 	oss << response.version << " " << response.status_code << " " << response.status_text << "\r\n";
// 	for (std::map<std::string, std::string>::const_iterator it = response.headers.begin(); it != response.headers.end(); ++it)
// 	{
// 		oss << it->first << ": " << it->second << "\r\n";
// 	}
// 	oss << "\r\n";
// 	oss << response.body;

// 	return oss.str();
// }

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