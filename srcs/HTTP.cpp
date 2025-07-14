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

const LocationConfig* locate(const HttpRequest &request, const ServerConfig &serverConfig)
{
	// Find the longest matching location prefix
	const LocationConfig* bestMatch = 0;
	size_t bestLen = 0;

	for (size_t k = 0; k < serverConfig.locations.size(); ++k)
	{
		const std::string& locName = serverConfig.locations[k].name;
		if (request.path.compare(0, locName.size(), locName) == 0)
		{
			// Ensure match is on a path boundary
			if ((locName.size() == request.path.size()) ||
				(request.path[locName.size()] == '/') )
			{
				if (locName.size() > bestLen)
				{
					bestLen = locName.size();
					bestMatch = &serverConfig.locations[k];
				}
			}
		}
	}
	return bestMatch;
}

HttpResponse generateResponse(const HttpRequest &request, const ServerConfig &serverConfig)
{
	HttpResponse response;

	// Logger::debug("req: " + request.path);

	const LocationConfig* location = locate(request, serverConfig);
	if (!location)
		return (generateErrorResponse(NOTFOUD, serverConfig.error_pages_dir, request.version));
	Logger::debug(*location);
	for (size_t i = 0; i < serverConfig.locations.size(); i++)
	{
		// Logger::debug("locs: " + serverConfig.locations[i].name);
		// Logger::debug("path: " + serverConfig.root + serverConfig.locations[i].root);
	}

	std::string path = resolvePath(request, serverConfig);
	// Logger::debug(path);

	return response;
}
