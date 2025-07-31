#include "HTTP.hpp"
#include "Config.hpp"
#include "Logger.hpp"

// Helper function to find the best matching location for a given path
const LocationConfig *findBestLocation(const std::string &path, const ServerConfig &server)
{
	size_t match = path.find('/', 1);

	if (match == std::string::npos)
	{
		for (size_t i = 0; i < server.locations.size(); i++)
		{
			if (path == server.locations[i].name)
				return &server.locations[i];
		}
		return server.defaultLocation;
		
	}
	else
	{
		for (size_t i = 0; i < server.locations.size(); i++)
		{
			if (!server.locations[i].name.compare(0, match, path))
				return &server.locations[i];
		}
		return NULL;
	}
	return NULL;
}

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


// POST /users HTTP/1.1
// Host: example.com
// Content-Type: application/x-www-form-urlencoded
// Content-Length: 49

// name=FirstName+LastName&email=bsmth%40example.com
HttpRequest parseRequest(const std::string &rawRequest, const ServerConfig &server)
{
	HttpRequest request;
	std::istringstream stream(rawRequest);
	std::string line;

	// Parse the request line (first line)
	if (!std::getline(stream, line))
		throw std::runtime_error("Invalid HTTP request: empty request");

	// Remove carriage return if present
	if (!line.empty() && line[line.length() - 1] == '\r')
		line.erase(line.length() - 1);

	std::istringstream requestLine(line);
	std::string methodStr, uri, version;

	if (!(requestLine >> methodStr >> uri >> version))
		throw std::runtime_error("Invalid HTTP request line");

	// Parse method
	request.method = methodStr;

	request.version = version;

	// Parse URI and query parameters
	size_t queryPos = uri.find('?');
	if (queryPos != std::string::npos)
	{
		request.path = uri.substr(0, queryPos);
		std::string queryString = uri.substr(queryPos + 1);
		request.query_params = parse_query(queryString);
	}
	else
	{
		request.path = uri;
	}

	// Parse headers
	while (std::getline(stream, line) && !line.empty())
	{
		// Remove carriage return if present
		if (!line.empty() && line[line.length() - 1] == '\r')
			line.erase(line.length() - 1);

		if (line.empty())
			break; // Empty line indicates end of headers

		size_t colonPos = line.find(':');
		if (colonPos != std::string::npos)
		{
			std::string headerName = line.substr(0, colonPos);
			std::string headerValue = line.substr(colonPos + 1);

			// Trim whitespace from header value
			size_t start = headerValue.find_first_not_of(" \t");
			if (start != std::string::npos)
			{
				size_t end = headerValue.find_last_not_of(" \t");
				headerValue = headerValue.substr(start, end - start + 1);
			}
			else
			{
				headerValue = "";
			}
			request.headers[headerName] = headerValue;
		}
	}

	// Parse body (everything after headers)
	std::string bodyLine;
	std::ostringstream bodyStream;
	while (std::getline(stream, bodyLine))
	{
		bodyStream << bodyLine << "\n";
	}
	request.body = bodyStream.str();

	// Remove trailing newline if present
	if (!request.body.empty() && request.body[request.body.length() - 1] == '\n')
		request.body.erase(request.body.length() - 1);

	// Find the best matching location for this request path
	request.best_location = findBestLocation(request.path, server);
	if (request.best_location)
		request.path = server.root + request.best_location->root + request.path.substr(request.best_location->name.length());
	return request;
}