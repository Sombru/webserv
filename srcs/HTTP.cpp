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

HttpRequest parseRequest(const std::string &raw_request, const ServerConfig &config)
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

HttpResponse recieveResponse(const ServerConfig &config)
{
	HttpResponse responceRecieved;
	std::string fullPath;

	// if (config.root.c_str() == "www/portfolio/" &&
	// 	config.index.c_str() == "index.html")
	// {
	fullPath = config.root;	  // path directory
	fullPath += config.index; // adding index directory
	std::ifstream file(fullPath.c_str());

	Logger::debug("working");
	if (file.is_open())
	{
		if (config.root == "www/portfolio/" &&
			config.index == "index.html")
		{
			std::stringstream body_stream;
			body_stream << file.rdbuf();
			responceRecieved.body = body_stream.str();
			responceRecieved.status_code = 200;
			responceRecieved.status_text = "OK";
			responceRecieved.headers["Content-Type"] = "text/html";
			{
				Logger::info("Proper page found\n");
				std::ostringstream len_stream;
				len_stream << responceRecieved.body.length();
				responceRecieved.headers["Content-Length"] = len_stream.str();
				return (responceRecieved);
			}
		}
		// if () file or error

		else
		{
			std::cout << "Not ready yet\n";
			// run error page
		}
	}
	//else
	//{ // error opening file -> open page error
	//}
	Logger::debug("Not working yet");
	return (responceRecieved);
}
