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
		return buildErrorRespose(404);
	response.body = buffer;

	addHeaders("Content-Type", getMimeType(fsTarget));
	addHeaders("Content-Lenght", intToString(response.body.length()));
}

void HTTP::buildResponse(int code)
{
	response.status_code = code;
	response.status_text = getStatusText(code);
}

void HTTP::redirect(const std::string &returnPath)
{
	buildResponse(302);
	addHeaders("Location", returnPath);
}

