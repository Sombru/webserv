#include "HTTP.hpp"
#include "Utils.hpp"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

// make serverLoc to always have location to access +
// make /login location that will redirect you to logind page for cookies bonus part +
// add fsIndex to location have easy access of location's indexes +
// autoinedx +
// autoindex allowes to delte files if DELETE method is present in location +
// CGI
// fix POST +
// fix DELETE not refreshing the page on delete +
// fix permanent buffering
// memory (signals!!!!!!!!!!)

HTTP::HTTP(ServerConfig &serverConfig)
	: serverConfig(serverConfig)
{
	response.version = HTTP_VERSION;
	data = NULL;
}

HTTP::HTTP(const HTTP &other)
	: data(other.data), 
	  serverConfig(other.serverConfig),
	  request(other.request),
	  response(other.response)
{

}

HTTP &HTTP::operator=(const HTTP &other)
{
	if (this != &other)
	{
		data = other.data;
		// serverConfig = other.serverConfig;
		request = other.request;
		response = other.response;
	}
	return *this;
}

HTTP::~HTTP()
{
	if (data)
	{
		delete[] data;
		data = NULL;
	}
}

void HTTP::parseRequest(const std::string &rawRequest)
{
	request.best_location = LocationConfig();
	request.body.clear();
	request.cookies.clear();
	request.headers.clear();
	request.method.clear();
	request.path.clear();
	request.query_params.clear();
	request.query_string.clear();
	request.target_file.clear();
	request.version.clear();

	response.body.clear();
	response.headers.clear();
	response.status_code = 0;
	response.status_text.clear();
	response.version.clear();

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
	std::string cookieHeader;
	if (request.headers.find("Cookie") != request.headers.end())
	{
		cookieHeader = request.headers["Cookie"];
		std::istringstream cookieStream(cookieHeader);
		std::string cookie;
		while (std::getline(cookieStream, cookie, ';'))
		{
			size_t eqPos = cookie.find('=');
			if (eqPos != std::string::npos)
			{
				std::string name = cookie.substr(0, eqPos);
				std::string value = cookie.substr(eqPos + 1);
				// Trim whitespace
				name.erase(0, name.find_first_not_of(" "));
				name.erase(name.find_last_not_of(" ") + 1);
				request.cookies[name] = value;
			}
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
	const LocationConfig *bestMatch = 0;
	size_t longestMatch = 0;
	if (request.path.empty())
		request.path = '/';

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
	request.best_location = *bestMatch;
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
			DEBUG("No Connection header, defaulting to keep-alive for HTTP/1.1");
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
	// ensure response version is set so status-line is valid
	response.version = HTTP_VERSION;
	std::string fsPath = resolveRequestPath();
	// call before methods
	handleConnectionHeader();
	if (!handleSession())
		return redirect("/login");
	if (request.body.size() > serverConfig.clientMaxBodySize)
		return buildErrorRespose(413);
	if (!methodAllowed(request.method))
		return buildErrorRespose(405);
	if (!request.best_location.returnPath.empty())
		return redirect(request.best_location.returnPath);
	if (isCgiScrit())
		return executeCgi(fsPath, request.best_location.cgi[getFileExtension(request.path)], request.body); 
	if (request.method == "GET")
		GET(fsPath);
	else if (request.method == "POST")
		POST();
	else if (request.method == "DELETE")
		DELETE(fsPath);
	else
		buildErrorRespose(405);
}

bool HTTP::handleSession()
{
	if (!hasLoginLocation(serverConfig.locations))
		return true; 
	if ((request.cookies.find("logged_in") != request.cookies.end() || request.cookies["logged_in"] == "true"))
		return true;
	if (request.best_location.path == "/login")
		return true;
	return false;
}

// Chunked transfer encoding methods
bool HTTP::processChunkData(const std::string& chunkData)
{
	chunkBuffer += chunkData;
	
	// Check if we have a complete chunk
	size_t crlfPos = chunkBuffer.find("\r\n");
	if (crlfPos == std::string::npos)
		return false; // Need more data for chunk size
	
	// Extract chunk size (hexadecimal)
	std::string chunkSizeLine = chunkBuffer.substr(0, crlfPos);
	
	// Remove any chunk extensions (after semicolon)
	size_t semicolonPos = chunkSizeLine.find(';');
	if (semicolonPos != std::string::npos)
		chunkSizeLine = chunkSizeLine.substr(0, semicolonPos);
	
	// Parse hexadecimal chunk size
	size_t chunkSize;
	std::istringstream iss(chunkSizeLine);
	iss >> std::hex >> chunkSize;
	
	if (iss.fail())
		return false; // Invalid chunk size
	
	// Check if we have the complete chunk data (size + data + CRLF)
	size_t requiredLength = crlfPos + 2 + chunkSize + 2; // size\r\n + data + \r\n
	if (chunkBuffer.length() < requiredLength)
		return false; // Need more data
	
	if (chunkSize == 0)
	{
		// Last chunk - request is complete
		// Look for trailing headers (optional)
		size_t trailingHeadersEnd = chunkBuffer.find("\r\n\r\n", crlfPos + 2);
		if (trailingHeadersEnd == std::string::npos)
			return false; // Need final CRLF
		
		// Chunked request is complete
		return true;
	}
	else
	{
		// Extract chunk data (skip size line and CRLF)
		std::string chunkDataPart = chunkBuffer.substr(crlfPos + 2, chunkSize);
		request.body += chunkDataPart;
		
		// Remove processed chunk from buffer
		chunkBuffer = chunkBuffer.substr(requiredLength);
		
		// Generate 202 Accepted response for this chunk
		generateChunkAcceptedResponse();
		return false; // More chunks expected
	}
}

void HTTP::generateChunkAcceptedResponse()
{
	response.status_code = 202;
	response.status_text = "Accepted";
	response.version = HTTP_VERSION;
	response.headers.clear();
	response.headers["Content-Length"] = "0";
	response.headers["Connection"] = "close";
	response.body = "";
}

bool HTTP::isChunkComplete(const std::string& data)
{
	// Check if data contains a complete chunk (size + data + CRLF)
	size_t crlfPos = data.find("\r\n");
	if (crlfPos == std::string::npos)
		return false;
	
	std::string chunkSizeLine = data.substr(0, crlfPos);
	size_t chunkSize;
	std::istringstream iss(chunkSizeLine);
	iss >> std::hex >> chunkSize;
	
	if (iss.fail())
		return false;
	
	// Check if we have complete chunk
	size_t requiredLength = crlfPos + 2 + chunkSize + 2;
	return data.length() >= requiredLength;
}

std::string HTTP::extractChunkSize(const std::string& data)
{
	size_t crlfPos = data.find("\r\n");
	if (crlfPos == std::string::npos)
		return "";
	
	return data.substr(0, crlfPos);
}

std::string HTTP::extractChunkData(const std::string& data, size_t chunkSize)
{
	size_t crlfPos = data.find("\r\n");
	if (crlfPos == std::string::npos)
		return "";
	
	size_t dataStart = crlfPos + 2;
	if (data.length() < dataStart + chunkSize)
		return "";
	
	return data.substr(dataStart, chunkSize);
}

