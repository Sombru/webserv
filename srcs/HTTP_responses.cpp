#include "HTTP.hpp"

void HTTP::buildErrorRespose(int code)
{
	response.status_code = code;
	response.status_text = getStatusText(code);
	response.body = loadErrorPage(code);
	response.headers["Content-Type"] = getMimeType(serverConfig.root + "/" + serverConfig.errorPage);
	response.headers["Content-Length"] = intToString(response.body.size());
}


void HTTP::buildResponse(int code, std::string &fsTarget)
{
	response.status_code = code;
	response.status_text = getStatusText(code);
	std::string buffer = readFile(fsTarget);
	if (buffer == BADFILE)
		return buildErrorRespose(500);
	response.body = buffer;
	response.headers["Content-Type"] = getMimeType(fsTarget);
	response.headers["Content-Length"] = intToString(response.body.size());
}

void HTTP::buildResponse(int code)
{
	response.status_code = code;
	response.status_text = getStatusText(code);
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