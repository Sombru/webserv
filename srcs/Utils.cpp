#include "Utils.hpp"
#include "Logger.hpp"
#include "Webserv.hpp"


// gets the MIME type based on file extension
std::string getMimeType(const std::string &path)
{
	size_t dotPos = path.find_last_of('.');
	if (dotPos == std::string::npos)
		return "application/octet-stream";

	std::string extension = path.substr(dotPos);

	if (extension == ".html" || extension == ".htm")
		return "text/html";
	else if (extension == ".css")
		return "text/css";
	else if (extension == ".js")
		return "application/javascript";
	else if (extension == ".png")
		return "image/png";
	else if (extension == ".jpg" || extension == ".jpeg")
		return "image/jpeg";
	else if (extension == ".gif")
		return "image/gif";
	else if (extension == ".pdf")
		return "application/pdf";
	else if (extension == ".txt")
		return "text/plain";
	else
		return "application/octet-stream";
}

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