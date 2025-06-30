#include "ParserManager.hpp"
#include "HttpResponseBuilder.hpp"
#include "Client.hpp"
#include "Logger.hpp"
#include <iostream>

// ParserManager::ParserManager(ParseType type, const std::string& filename)
// {
// 	switch(type)
// 	{
// 		case(CONFIG):
// 			config.parse(filename);
// 			break;
// 		case(RESPONSE):
// 			std::cout << "here is the rule for parsing a response" << std::endl;
// 			break;
// 		case(REQUEST):
// 			std::cout << "here is the rule for parsing a request" << std::endl;
// 			break;
// 	}
// }

int ParserManager::parseRequest(const Client& client)	
{
	std::istringstream stream(client.getRaw_request());
	std::string line;

	// Parse the request line: "GET /path?query=string HTTP/1.1"
	if (std::getline(stream, line))
	{
		std::istringstream line_stream(line);
		line_stream >> this->request.method;

		std::string full_path;
		line_stream >> full_path;

		size_t q = full_path.find('?');
		if (q != std::string::npos)
		{
			this->request.path = full_path.substr(0, q);
			this->request.query_string = full_path.substr(q + 1);
			this->request.query_params = parse_query(this->request.query_string);
		}
		else
		{
			this->request.path = full_path;
		}

		line_stream >> this->request.version;
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
			this->request.headers[key] = value;
		}
	}
	std::cout << this->request;
	// req.path = INDEX; // this will be something else idk what for now
	return 0;
}

std::string HttpResponse::serialize() const {
	std::ostringstream oss;

	oss << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
		oss << it->first << ": " << it->second << "\r\n";
	}
	oss << "\r\n";
	oss << body;

	return oss.str();
}

HttpResponse HttpResponseBuilder::build(const HttpRequest& req) {
	HttpResponse res;
	std::string full_path = "webpages";	// the current directory

	if (req.path == "/")
		full_path +="/index.html";
	else
		full_path += req.path;			// appent the requested path

	std::ifstream file(full_path.c_str());

	if (!file.is_open())
	{
		res.status_code = 404;
		res.status_text = "Not Found";
		res.body = "<h1>404 Not Found</h1>";
		res.headers["Content-Type"] = "text/html";

	} else {
		std::stringstream body_stream;
		body_stream << file.rdbuf();
		res.body = body_stream.str();
		res.status_code = 200;
		res.status_text = "OK";
		res.headers["Content-Type"] = "text/html";
	}
	std::ostringstream len_stream;
	len_stream << res.body.length();
	res.headers["Content-Length"] = len_stream.str();
	return res;
}

int ParserManager::buildResponse(Socket& webserv, const Client& client)
{
	// Build response from parsed request using your new modular builder
	this->response = HttpResponseBuilder::build(this->request);

	// Debug: print readable response
	std::cout << this->response;
	
	// Serialize to raw HTTP string and send it
	std::string raw_response = this->response.serialize();
	webserv.setResponse(raw_response);
	webserv.response(client); // <- actual, connected client

	return (0);
}

std::ostream &operator<<(std::ostream &os, const HttpResponse &res)
{
	os << std::endl;
	os << "=== HTTP RESPONSE ===\n";
	os << "Version:  " << res.version << "\n";
	os << "Status_code:    " << res.status_code << "\n";
	os << "Status_text: " << res.status_text << "\n";
	os << "Body: " << res.body << "\n";

	os << "Headers:\n";
	for (strHmap::const_iterator it = res.headers.begin(); it != res.headers.end(); ++it)
	{
		os << "  " << it->first << ": " << it->second << "\n";	
	}

	os << "====================\n";
	return os;
}