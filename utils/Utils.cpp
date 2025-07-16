#include "Webserv.hpp"
#include "HTTP.hpp"

// takes a path to a file and returns it contents or "BAD" if empty
std::string readFile(const std::string &path)
{
	std::ifstream file(path.c_str());
	if (!file.good())
		return ("BAD");
	std::stringstream buffer;

	buffer << file.rdbuf();

	if (buffer.str().empty())
		return ("BAD");
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

// gets directory contets in a specified location of a server
std::vector<std::string> getLocationContents(const std::string &server_root, const std::string &location_root)
{
	std::vector<std::string> contents;
	std::string full_path = server_root + location_root;

	DIR *dir = opendir(full_path.c_str());
	if (!dir)
		return contents; // Could not open directory

	struct dirent *entry;
	contents.reserve(10); // buffer
	while ((entry = readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;
		if (name != "." && name != "..")
			contents.push_back(name);
	}
	closedir(dir);
	return contents;
}

// gets directory contets in a specified location of a server
std::vector<std::string> getLocationContents(const std::string& path)
{
	std::vector<std::string> contents;

	DIR *dir = opendir(path.c_str());
	if (!dir)
		return contents; // Could not open directory

	struct dirent *entry;
	contents.reserve(10); // buffer
	while ((entry = readdir(dir)) != NULL)
	{
		std::string name = entry->d_name;
		if (name != "." && name != "..")
			contents.push_back(name);
	}
	closedir(dir);
	return contents;
}