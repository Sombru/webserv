#include "Utils.hpp"
#include "Logger.hpp"
#include "Webserv.hpp"

// Read file in binary mode for images and other binary content
std::string readFileBinary(const std::string &path)
{
	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file.is_open())
		return BADFILE;

	std::ostringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

// takes a path to a file and returns it contents
/// returns file contens, BADFILE if cant open/read/empty
std::string readFile(const std::string &path)
{
	std::ifstream file(path.c_str());
	std::stringstream buffer;

	buffer << file.rdbuf();

	if (buffer.str().empty())
		return (BADFILE);
	return (buffer.str());
}

std::string intToString(int n)
{
	std::stringstream ss;
	ss << n;
	return ss.str();
}

std::string getTimestamp()
{
	time_t now = time(0);
	tm *localtm = localtime(&now);

	char buf[20];
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtm);

	return (std::string(buf));
}

bool is_directory(const std::string &path)
{
	struct stat info;
	if (stat(path.c_str(), &info) != 0)
	{
		return false; // cannot access path
	}
	return (info.st_mode & S_IFDIR) != 0;
}

bool hasLoginLocation(const std::vector<LocationConfig> &locations)
{
	for (size_t i = 0; i < locations.size(); ++i)
	{
		if (locations[i].path == "/login")
			return true;
	}
	return false;
}

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