#include "ParserManager.hpp"
#include "Logger.hpp"

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
