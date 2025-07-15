#include "GET.hpp"
#include "Logger.hpp"

// populate GET request with data needed
GET::GET(HttpResponse &response_src, const std::string &requestTarget_src, const LocationConfig *locaiont_src, const ServerConfig &server_src, const std::string& version_src)
	: response(response_src), requestTargert(requestTarget_src), location(locaiont_src), server(server_src), version(version_src)
{

}

GET::~GET()
{
}


void GET::buildResponse()
{
	std::string fs_path; // filesystem path
	if (requestTargert.empty() == false)
	{
		fs_path = server.root + location->root + requestTargert;
		response.body = readFile(fs_path);
		if (response.body == "BAD")
			this->response = generateErrorResponse(NOTFOUD, this->server.error_pages_dir, this->version);
		else
		{
			this->response.status_code = OK; 
			this->response.status_text = "OK";
			this->response.version = this->version;
		}
		return ;
	}
	else 
		fs_path = server.root + location->root;
	
	// serve index generate auto_indexing
}

HttpResponse &GET::getResponse()
{
	return this->response;
}