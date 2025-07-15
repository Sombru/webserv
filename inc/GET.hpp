#pragma once

#include "Webserv.hpp"
#include "HTTP.hpp"
#include "Config.hpp"

class GET
{
private:
	struct HttpResponse& response;

	const std::string& requestTargert;
	const LocationConfig* location;

	const ServerConfig& server;
	const std::string& version;
public:
	GET(HttpResponse &response_src, const std::string &requestTarget_src, const LocationConfig *locaiont_src, const ServerConfig &server_src, const std::string& version_src);
	~GET();

	void buildResponse();
	HttpResponse& getResponse();

};


