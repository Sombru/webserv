#include "Logger.hpp"
#include "Webserv.hpp"
#include "Utils.hpp"

// takes a path to a file and returns it contents
/// @returns file contens, BADFILE if cant open/read/empty
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
