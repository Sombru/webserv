#include "HTTP.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include "GET.hpp"

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
	
	std::string body;
    if (request.headers.count("Content-Length"))
    {
        int content_length = std::atoi(request.headers["Content-Length"].c_str());
        if (content_length > 0)
        {
            body.resize(content_length);
            stream.read(&body[0], content_length);
            request.body = body;
        }
    }

    return (request);
}

// Finds the most appropriate LocationConfig for the given HttpRequest by matching the request path
// or NULL if no match
const LocationConfig *locate(const HttpRequest &request, const ServerConfig &serverConfig)
{
	size_t len = request.path.find_last_of('/');
	if (len == 0)
		return &serverConfig.locations[serverConfig.default_location_index];
	for (size_t k = 0; k < serverConfig.locations.size(); k++)
	{
		if (serverConfig.locations[k].name == "/")
			continue ;
		const std::string &locName = serverConfig.locations[k].name;
		if (request.path.compare(0, locName.size(), locName) == 0)
			return &serverConfig.locations[k];
	}
	return NULL;
}

HttpResponse generateResponse(const HttpRequest &request, const ServerConfig &serverConfig)
{
	struct HttpResponse response;

	std::string requestTarget = request.path.substr(request.path.find_last_of('/')+1);
	const LocationConfig *location = locate(request, serverConfig);

	if (!location)
		return generateErrorResponse(NOTFOUD, serverConfig.error_pages_dir, request.version);

	if (request.method == "GET")
	{
		GET get(response, requestTarget, location, serverConfig, request.version);
		get.buildResponse();
		return get.getResponse();
	}	
	return response;
}

// get a request body
// split in into GET POST DELETE class
// pass response as a reffernce and handle it internally
