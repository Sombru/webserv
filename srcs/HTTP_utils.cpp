#include "HTTP.hpp"

int HTTP::methodAllowed(std::string &requestMethod)
{
	int res = false;

	for (size_t i = 0; i < request.best_location.allowedMethods.size(); ++i)
	{
		if (request.best_location.allowedMethods[i] == requestMethod)
			res = true;
	}
	return res;	
}

void HTTP::addHeaders(const std::string &header, const std::string &value)
{
	response.headers[header] = value;
}

std::string HTTP::getStatusText(int code)
{
	switch (code)
	{
	case 200:
		return "OK";
	case 302:
		return "Found";
	case 400:
		return "Bad Request";
	case 403:
		return "Forbidden";
	case 404:
		return "Not Found";
	case 405:
		return "Method not allowed";
	case 413:
		return "Payload Too Large";
	case 500:
		return "Internal Server Error";
	case 502:
		return "Bad Gateway";
	default:
		return "Error";
	}
}

// Helper method to replace placeholders in error pages
std::string HTTP::replacePlaceHolders(std::string source,
									const std::string &from,
									const std::string &to)
{
	size_t pos;
	while ((pos = source.find((from)) != std::string::npos))
		source.replace(pos, from.size(), to);
	return source;
}

// returns a fileSystem path of a request
std::string HTTP::resolveRequestPath()
{
	return request.best_location.fs_path + "/" + request.path.substr(request.best_location.path.size());
}