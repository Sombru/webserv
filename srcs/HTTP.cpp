#include "HTTP.hpp"

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
		int content_length = std::atoi(request.headers["Content-Length"].c_str());
		if (content_length > 0)
		{
			body.resize(content_length);
			stream.read(&body[0], content_length);
			request.body = body;
		}
	}
}

std::map<std::string, std::string> HTTP::parseQuery(const std::string &query_string)
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
	{
		DEBUG("Found matching location: " + bestMatch->path);
	}
	else
	{
		DEBUG("No matching location found for path: " + request.path);
	}
}

void HTTP::generateResponse()
{
	// if (request.method == "GET")
	// 	GET();
	// else
	// 	ERROR("Unhandled request method");
}