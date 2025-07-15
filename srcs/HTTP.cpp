#include "HTTP.hpp"
#include "Config.hpp"
#include "Logger.hpp"

std::map<std::string, std::string> parse_query(const std::string &query_string)
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

HttpRequest parseRequset(const std::string &raw_request, const ServerConfig &config)
{
	HttpRequest request;
	std::istringstream stream(raw_request);
	std::string line;

	(void)config;
	// Parse the request line: "GET /path?query=string HTTP/1.1"
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
			request.query_params = parse_query(request.query_string);
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
			request.headers[key] = value;
		}
	}
	// req.path = INDEX; // this will be something else idk what for now
	return (request);
}

std::string resolvePath(const HttpRequest &request, const ServerConfig &serverConfig)
{
	// Logger::debug(serverConfig.root + request.path);
	std::string fullPath;

	for (size_t i = 0; i < serverConfig.locations.size(); i++)
	{
		if (!serverConfig.locations[i].name.compare(request.path))
			fullPath = serverConfig.root + serverConfig.locations[i].name;
	}

	return fullPath;
}

// Finds the most appropriate LocationConfig for the given HttpRequest by matching the request path
// or NULL if no match
const LocationConfig *locate(const HttpRequest &request, const ServerConfig &serverConfig)
{
	size_t len = request.path.find_last_of('/');
	if (len == std::string::npos || len == 0)
		return &serverConfig.locations[serverConfig.default_location_index];
	for (size_t k = 0; k < serverConfig.locations.size(); k++)
	{
		if (serverConfig.locations[k].name == "/")
			continue;
		const std::string &locName = serverConfig.locations[k].name;
		if (request.path.compare(0, locName.size(), locName) == 0)
			return &serverConfig.locations[k];
	}
	return NULL;
}

HttpResponse generateResponse(const HttpRequest &request, const ServerConfig &serverConfig)
{
	HttpResponse response;

	Logger::debug("req: " + request.path);

	Logger::debug(request.path.substr(request.path.find_last_of('/')));
	const LocationConfig *location = locate(request, serverConfig);

	// std::string requestFile = request.path.substr(request.path.find_last_of('/')+);
	if (location)
		Logger::debug(*location);
	else
		return generateErrorResponse(NOTFOUND, serverConfig.error_pages_dir, request.version);
		// Logger::debug(requestFile);

	// std::string path = resolvePath(request, serverConfig);
	// Logger::debug(path);

	return response;
}
