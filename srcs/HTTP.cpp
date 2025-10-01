#include "HTTP.hpp"
#include "Utils.hpp"

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

HTTP::HTTP(const std::string &rawRequest, const ServerConfig &serverConfig)
	: rawRequest(rawRequest), serverConfig(serverConfig)
{
	// Initialize request and response
	request.method = "";
	request.path = "";
	request.target_file = "";
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
	// DEBUG("Found matching location: " + bestMatch->path);
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
	if (!methodAllowed(request.method))
		return buildErrorRespose(405);
	if (!request.best_location.returnPath.empty())
		return redirect(request.best_location.returnPath);
	if (!handleSession())
		return redirect("/login");
	std::string fsPath = resolveRequestPath();

	// DEBUG(fsPath);
	// DEBUG(request.best_location.fs_uploadDir);
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

// Helper method to load and prepare error pages
std::string HTTP::loadErrorPage(int code)
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
		return "<html><body><h1>" + intToString(code) + " " + getStatusText(code) +
			   "</h1></body></html>";
	}

	// Replace placeholders in the error page
	errorBody = replacePlaceHolders(errorBody, "{{code}}", intToString(code));
	errorBody = replacePlaceHolders(errorBody, "{{status_text}}", getStatusText(code));
	return errorBody;
}

