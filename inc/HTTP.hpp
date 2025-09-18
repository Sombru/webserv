#pragma once
#include "Logger.hpp"
#include "Server.hpp"
#include "Webserv.hpp"

#define HTTP_VERSION "HTTP/1.1"

struct HttpRequest
{
	std::string method;					 // e.g. GET
	std::string path;					 // e.g. /about.html
	std::string target_file;			 // e.g. about.html
	const LocationConfig *best_location; // e.g. /files/
	std::string query_string;			 // e.g ?alice=18
	std::string version;				 //  e.g. "HTTP/1.1"
	std::map<std::string, std::string> query_params; // e.g. query_params["alice"] == 18
	std::map<std::string, std::string> headers; // e.g. headers["Authorization"] == <browser>
	std::string body;
};

struct HttpResponse
{
	int status_code;		 // e.g. 200
	std::string status_text; // e.g. "OK"
	std::string version;	 // e.g. "HTTP/1.1"
	std::map<std::string, std::string> headers;	  // e.g. headers["Content-Length"] == body.size()
	std::string body; // e.g. Hello, world!
	bool is_download_file; // tracking if recognize downloadable file or web page
};

class HTTP
{
private:
	std::string loadErrorPage(int code, const std::string &statusText);
	void replacePlaceholders(std::string &content, int code,
							 const std::string &statusText);
	std::string getStatusText(int code);
	std::string resolveRequestPath();
	std::string getMimeType(const std::string &path);
	const std::string &rawRequest;
	const ServerConfig &serverConfig;

	std::map<std::string, std::string>
	parseQuery(const std::string &query_string);

	void findBestLocation();
	void handleConnectionHeader();

	// Error page rendering helpers
	std::string replaceAllOccurrences(std::string source,
									  const std::string &from,
									  const std::string &to);

	void GET();
	void POST();
	void DELETE();

public:
	char *data;
	HttpRequest request;
	HttpResponse response;

	HTTP(const std::string &rawRequest, const ServerConfig &serverConfig);
	void parseRequest();
	void generateResponse();

	~HTTP();
};
