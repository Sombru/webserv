#include "HTTP.hpp"
#include "Utils.hpp"

HTTP::HTTP(const std::string &rawRequest, const ServerConfig &serverConfig)
	: rawRequest(rawRequest), serverConfig(serverConfig)
{
	// Initialize request and response
	request.method = "";
	request.path = "";
	request.target_file = "";
	request.best_location = NULL;
	request.query_string = "";
	request.version = "";

	response.status_code = 200;
	response.status_text = "OK";
	response.version = HTTP_VERSION;
	response.is_download_file = false;

	data = NULL;
}

HTTP::~HTTP()
{
	if (data)
	{
		delete[] data;
		data = NULL;
	}
}

void HTTP::parseRequest()
{
	std::istringstream stream(rawRequest);
	std::string line;

	if (std::getline(stream, line))
	{
		std::istringstream line_stream(line);
		line_stream >> request.method;

		std::string full_path;
		line_stream >> full_path;

		size_t q = full_path.find('?');
		if (q != std::string::npos)
		{
			request.path = full_path.substr(0, q);
			request.query_string = full_path.substr(q + 1);
			request.query_params = parseQuery(request.query_string);
		}
		else
		{
			request.path = full_path;
		}

		line_stream >> request.version;
	}

	while (std::getline(stream, line) && line != "\r")
	{
		size_t colon = line.find(':');
		if (colon != std::string::npos)
		{
			std::string key = line.substr(0, colon);
			std::string value = line.substr(colon + 1);
			if (!value.empty() && value[0] == ' ')
				value = value.substr(1);
			if (!value.empty() && value[value.length() - 1] == '\r')
				value = value.substr(0, value.length() - 1);
			// request.keys.push_back(key);
			request.headers[key] = value;
		}
	}
	findBestLocation();

	std::string body;
	if (request.headers.count("Content-Length"))
	{
		int content_length =
			std::atoi(request.headers["Content-Length"].c_str());
		if (content_length > 0)
		{
			body.resize(content_length);
			stream.read(&body[0], content_length);
			request.body = body;
		}
	}
}

std::map<std::string, std::string>
HTTP::parseQuery(const std::string &query_string)
{
	std::map<std::string, std::string> params;
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

void HTTP::findBestLocation()
{
	const LocationConfig *bestMatch = NULL;
	size_t longestMatch = 0;

	// Find the location with the longest matching path prefix
	for (size_t i = 0; i < serverConfig.locations.size(); ++i)
	{
		const LocationConfig &location = serverConfig.locations[i];

		// Check if request path starts with location path
		if (request.path.find(location.path) == 0)
		{
			size_t matchLength = location.path.length();
			if (matchLength > longestMatch)
			{
				longestMatch = matchLength;
				bestMatch = &location;
			}
		}
	}

	request.best_location = bestMatch;
	if (bestMatch)
		DEBUG("Found matching location: " + bestMatch->path);
	else
		DEBUG("No matching location found for path: " + request.path);
}

void HTTP::handleConnectionHeader()
{
	// Check if client sent Connection header
	std::map<std::string, std::string>::iterator it =
		request.headers.find("Connection");

	if (it != request.headers.end())
	{
		std::string connectionValue = it->second;
		std::string lowerValue = connectionValue;
		for (size_t i = 0; i < lowerValue.length(); ++i)
			lowerValue[i] = std::tolower(lowerValue[i]);
		if (lowerValue == "close")
		{
			response.headers["Connection"] = "close";
			DEBUG("Setting Connection: close");
		}
		else if (lowerValue == "keep-alive")
		{
			response.headers["Connection"] = "keep-alive";
			DEBUG("Setting Connection: keep-alive");
		}
		else
		{
			response.headers["Connection"] = "close";
			DEBUG("Unknown Connection header value, defaulting to close");
		}
	}
	else
	{
		if (request.version == "HTTP/1.1")
		{
			response.headers["Connection"] = "keep-alive";
			DEBUG(
				"No Connection header, defaulting to keep-alive for HTTP/1.1");
		}
		else
		{
			response.headers["Connection"] = "close";
			DEBUG("No Connection header, defaulting to close for HTTP/1.0 or "
				  "unknown version");
		}
	}
}

void HTTP::generateResponse()
{
	// call before methods
	handleConnectionHeader();

	if (request.method == "GET")
		GET();
	else if (request.method == "POST")
		POST();
	else if (request.method == "DELETE")
		{
			DEBUG("DELETE method called");
			DELETE();
		}
	else
	{
		// Minimal fallback: show error page with 405
		std::string body = readFile(serverConfig.errorPage);
		response.status_code = 405;
		response.status_text = "Method Not Allowed";
		response.body = (body == BADFILE) ? std::string("") : body;
		response.headers["Content-Type"] = "text/html";
		response.headers["Content-Length"] = intToString(response.body.size());
	}
}

// Helper method to replace placeholders in error pages
void HTTP::replacePlaceholders(std::string &content, int code,
							   const std::string &statusText)
{
	size_t pos;
	while ((pos = content.find("{{code}}")) != std::string::npos)
		content.replace(pos, 8, intToString(code));
	while ((pos = content.find("{{status_text}}")) != std::string::npos)
		content.replace(pos, 15, statusText);
}

// Helper method to load and prepare error pages
std::string HTTP::loadErrorPage(int code, const std::string &statusText)
{
	// Resolve error page path relative to server root if needed
	std::string errorPath = serverConfig.errorPage;
	if (!errorPath.empty() && errorPath[0] != '/')
	{
		std::string base = serverConfig.root;
		if (!base.empty() && base[base.size() - 1] == '/' &&
			!errorPath.empty() && errorPath[0] == '/')
			base.resize(base.size() - 1);
		else if (!base.empty() && base[base.size() - 1] != '/' &&
				 !errorPath.empty() && errorPath[0] != '/')
			base += "/";
		errorPath = base + errorPath;
	}
	// Try to load the server's error page
	std::string errorBody = readFile(errorPath);
	// If error page doesn't exist, use a simple fallback
	if (errorBody == BADFILE)
	{
		return "<html><body><h1>" + intToString(code) + " " + statusText +
			   "</h1></body></html>";
	}

	// Replace placeholders in the error page
	replacePlaceholders(errorBody, code, statusText);
	return errorBody;
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
