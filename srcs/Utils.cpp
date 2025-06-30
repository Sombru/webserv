#include "include/Webserv.hpp"

void openFile(std::ifstream& file, const std::string& filename)
{
    file.open(filename.c_str());
    if (!file.is_open())
        throw std::runtime_error("Failed to open file: " + filename);
}

strHmap parse_query(const std::string &query_string)
{
	strHmap params;
	std::istringstream ss(query_string);
	std::string pair;
	while (std::getline(ss, pair, '&'))
	{
		size_t eq = pair.find('=');
		if (eq != std::string::npos)
			params[pair.substr(0, eq)] = pair.substr(eq + 1);
		else
			params[pair] = ""; // no value
	}
	return params;
}

std::ostream &operator<<(std::ostream &os, const HttpRequest &req)
{
	os << "=== HTTP REQUEST ===\n";
	os << "Method:  " << req.method << "\n";
	os << "Path:    " << req.path << "\n";
	os << "Version: " << req.version << "\n";

	if (!req.query_string.empty())
	{
		os << "Query:   " << req.query_string << "\n";
		os << "Query Parameters:\n";
		for (strHmap::const_iterator it = req.query_params.begin(); it != req.query_params.end(); ++it)
		{
			os << "  " << it->first << " = " << it->second << "\n";
		}
	}

	os << "Headers:\n";
	for (strHmap::const_iterator it = req.headers.begin(); it != req.headers.end(); ++it)
	{
		os << "  " << it->first << ": " << it->second << "\n";	
	}

	os << "====================\n";
	return os;
}

