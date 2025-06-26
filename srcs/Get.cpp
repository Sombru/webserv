#include "Get.hpp"
#include "Webserv.hpp"

Get::Get(const HttpRequest& req)
: m_req(req)
{

}

Get::~Get()
{

}

std::string to_string(HttpResponse& res) ;

std::string Get::response() const
{
	
	std::ifstream index(m_req.path.c_str());
	if (index.bad())
		index.open(INDEX);

	// std::cout << m_req.path.c_str();
	// this will be proper HTTP response
	HttpResponse resopne;
	
	resopne.status_text = "OK";
	resopne.status_code = 200;
	resopne.headers = m_req.headers;
	std::stringstream page;
	
	page << index.rdbuf();
	std::string body = page.str();
	std::stringstream response;
	response<< "Content-Type: text/html\r\n"
			<< "Content-Length: " << body.size() << "\r\n"
			<< "\r\n"
			<< body;

	resopne.body = body;
	// std::cout << m_req.headers["Pragma"] << '\n';

	return (to_string(resopne));
}

std::string to_string(HttpResponse& res) 
{
	std::ostringstream oss;
	oss << res.version << " " << res.status_code << " " << res.status_text << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it = res.headers.begin(); it != res.headers.end(); ++it) {
		oss << it->first << ": " << it->second << "\r\n";
	}
	oss << "\r\n" << res.body;
	return oss.str();
}