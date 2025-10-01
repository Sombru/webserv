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
	if (response.headers.find(header) == response.headers.end())
		response.headers[header] = value;
	else
		response.headers[header] += "; " + value;
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
    if (from.empty()) 
        return source; 

    size_t pos = 0;
    while ((pos = source.find(from, pos)) != std::string::npos) 
	{
        source.replace(pos, from.length(), to);
        pos += to.length(); // move past the replaced content
    }
    return source;
}

// returns a fileSystem path of a request
std::string HTTP::resolveRequestPath()
{
	return request.best_location.fs_path + "/" + request.path.substr(request.best_location.path.size());
}

std::string HTTP::getMimeType(const std::string &path)
{
	size_t dotPos = path.find_last_of('.');
	if (dotPos == std::string::npos)
		return "application/octet-stream";

	std::string extension = path.substr(dotPos + 1); // skip the '.'

	if (extension.empty())
		return "application/octet-stream";
	// DEBUG(serverConfig.mimeTypes.at("text/html"));

	std::map<std::string, std::string>::const_iterator it =
		serverConfig.mimeTypes.find(extension);
	if (it != serverConfig.mimeTypes.end())
		return it->second;

	return "application/octet-stream";
}