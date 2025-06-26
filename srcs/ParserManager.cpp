#include "ParserManager.hpp"
#include "Client.hpp"
#include "Logger.hpp"

ParserManager::ParserManager(ParseType type, const std::string& filename)
{
	switch(type)
	{
		case(CONFIG):
			config.parse(filename);
			break;
		case(RESPONSE):
			std::cout << "here is the rule for parsing a response" << std::endl;
			break;
		case(REQUEST):
			std::cout << "here is the rule for parsing a request" << std::endl;
			break;
	}
}

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

int ParserManager::buildResponse(Socket& webserv)
{
	std::ifstream index(this->request.path.c_str());
	if (index.bad())
		index.open(INDEX);
	if (index.good())
		Logger::debug(INDEX);
	// this will be proper HTTP response
	this->response.status_text = "OK";
	this->response.status_code = 200;
	this->response.headers = this->request.headers;
	std::stringstream page;
	
	// page << index.rdbuf();
	// std::string body = page.str();
	// std::stringstream response_body;
	// response_body << "Content-Type: text/html\r\n"
	// 			  << "Content-Length: " << body.size() << "\r\n"
	// 		 	  << "\r\n"
	// 		 	  << body;

	// this->response.body = body;
	this->response.body = "HTTP/1.1 200 OK\r\n"
			"Content-Type: text/plain\r\n"
			"Content-Length: 13\r\n"
			"\r\n"
			"Hello, world!";
	std::cout << this->response.body;

	// std::ostringstream oss;
	// oss << this->response.version << " " << this->response.status_code << " " << this->response.status_text << "\r\n";
	// for (strHmap::const_iterator it = this->response.headers.begin(); it != this->response.headers.end(); ++it) {
	// 	oss << it->first << ": " << it->second << "\r\n";
	// }
	// oss << "\r\n" << this->response.body;
	// std::cout << this->response.body << '\n';
	webserv.setResponse(this->response.body);
	return (0);
}
