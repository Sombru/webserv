#include "Webserv.hpp"
#include "HTTP.hpp"

// takes a path to a file and returns it contents or "BAD" if empty
std::string readFile(const std::string& path)
{
    std::ifstream file(path.c_str());
    if (file.bad())
        return ("BAD");
    std::stringstream buffer;

    buffer << file.rdbuf();

    return (buffer.str());
}

std::string intToString(int n) {
    std::stringstream ss;
    ss << n;
    return ss.str();
}

std::string serialize(const HttpResponse& response)
{
	std::ostringstream oss;

	oss << response.version <<  " " << response.status_code << " " << response.status_text << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it = response.headers.begin(); it != response.headers.end(); ++it) {
		oss << it->first << ": " << it->second << "\r\n";
	}
	oss << "\r\n";
	oss << response.body;

	return oss.str();
}