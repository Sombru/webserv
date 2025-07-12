#include "HTTP.hpp"
#include "Config.hpp"
#include "Webserv.hpp"
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

HttpRequest parseRequset(const std::string& raw_request, const ServerConfig& config)
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



// serving the default page
HttpResponse recieveResponse(HttpRequest request, const ServerConfig& config) {
	HttpResponse response;

	// If it's just "/", then we use index.html
	std::string path = request.path;
	if (path == "/index.html") // if is already "/index.html"
		path = "/index.html";
	
	if (path == "/")
		path += config.index; // becomes "index.html"
	// Combine later with root
	response.fullPath = config.root + path;

	// Open the file
	std::ifstream file(response.fullPath.c_str());

	if (!file.is_open()) {
		response.fullPath = "www/errors/404.html";
		std::ifstream errorFile(response.fullPath.c_str());
		// read 404 page body
		std::stringstream buffer;
		buffer << errorFile.rdbuf();
		response.body = buffer.str();
		std::cout << "response.body is: " << response.body << std::endl;

		// ⚠️ 404 Not Found
		Logger::info("File not found");
		response.version = request.version;
		response.status_code = 404;
		response.status_text = "Not Found";
		response.headers["Content-Type"] = "text/html";
		response.headers["Content-length"] = intToString(response.body.length());
		std::cout << "PRINTING:\n"
				<< "response.status_code is " << response.status_code;
		std::cout << "\nresponse.body is " << response.body << std::endl; 
		return (response);
	}

	// If file is found
	std::stringstream buffer;
	buffer << file.rdbuf();
	// ⚠️ Add here if buffer length is too long open page 413.
	response.body = buffer.str();

	Logger::info("running recive request");
	response.version = request.version;
	response.status_code = 200;
	response.status_text = "OK";
	response.headers["Content-Type"] = "text/html";
	response.headers["Content-Length"] = intToString(response.body.length());

	std::cout << "PRINTING:\n"
			<< "response.status_code is " << response.status_code;
	std::cout << "\nresponse.body is " << response.body << std::endl; 
	return (response);
}

// sending the response back to the client - serializes the HTTP response
std::string serialize(HttpResponse &response) {
	std::ostringstream oss;

	oss << response.version << " " << response.status_code 
		<< " " << response.status_text << "\r\n";
	
	for (std::map<std::string, std::string>::const_iterator it =
		response.headers.begin(); it != response.headers.end(); ++it) {
			oss << it->first << ": " << it->second << "\r\n";
		}
	oss << "\r\n";
	oss << response.body;

	return (oss.str());
}

// response page if request body is too long:
HttpResponse tooLargeRequest(void) {

	HttpResponse response;
	response.fullPath = "www/errors/413.html";
	std::ifstream errorFile(response.fullPath.c_str());
	// read 413 page body
	std::stringstream buffer;
	buffer << errorFile.rdbuf();
	response.body = buffer.str();
	std::cout << "response.body is: " << response.body << std::endl;

	// ⚠️ 404 Payload Too Large
	Logger::info("Request body too large");
	response.version = "HTTP/1.1";
	response.status_code = 413;
	response.status_text = "Payload Too Large";
	response.headers["Content-Type"] = "text/html";
	response.headers["Content-length"] = intToString(response.body.length());
	std::cout << "PRINTING:\n"
			<< "response.status_code is " << response.status_code;
	std::cout << "\nresponse.body is " << response.body << std::endl; 
	return (response);
}