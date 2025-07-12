#include "Webserv.hpp"

// takes a path to a file and returns it contents or 0 if empty
std::string openFile(const std::string& path)
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

// strHmap parse_query(const std::string &query_string)
// {
// 	strHmap params;
// 	std::istringstream ss(query_string);
// 	std::string pair;
// 	while (std::getline(ss, pair, '&'))
// 	{
// 		size_t eq = pair.find('=');
// 		if (eq != std::string::npos)
// 			params[pair.substr(0, eq)] = pair.substr(eq + 1);
// 		else
// 			params[pair] = ""; // no value
// 	}
// 	return params;
// }

