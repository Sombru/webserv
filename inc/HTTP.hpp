#pragma once
#include "Logger.hpp"
#include "Server.hpp"
#include "Webserv.hpp"

#define HTTP_VERSION "HTTP/1.1"

struct HttpRequest
{
	std::string method;								 // e.g. GET
	std::string path;								 // e.g. /about.html
	std::string target_file;						 // e.g. about.html
	std::string query_string;						 // e.g ?alice=18
	std::string version;							 //  e.g. "HTTP/1.1"
	std::map<std::string, std::string> query_params; // e.g. query_params["alice"] == 18
	std::map<std::string, std::string> headers;		 // e.g. headers["Authorization"] == <browser>
	std::string body;
	std::map<std::string, std::string> cookies;
	LocationConfig best_location;					 // e.g. /files/
};

struct HttpResponse
{
	int status_code;							// e.g. 200
	std::string status_text;					// e.g. "OK"
	std::string version;						// e.g. "HTTP/1.1"
	std::map<std::string, std::string> headers; // e.g. headers["Content-Length"] == body.size()
	std::string body;							// e.g. Hello, world!
	bool is_download_file;						// tracking if recognize downloadable file or web page
};

class HTTP
{
  private:
	std::string resolveRequestPath();
	std::string getMimeType(const std::string &path);
	const std::string &rawRequest;
	const ServerConfig &serverConfig;

	std::map<std::string, std::string> parseQuery(const std::string &query_string);

	int methodAllowed(std::string &requestMethod);
	void redirect(const std::string &returnPath);

	void findBestLocation();
	void handleConnectionHeader();

	void buildResponse(int code, std::string &fsTarget);
	void buildErrorRespose(int code);
	void buildResponse(int code);

	void addHeaders(const std::string &header, const std::string &value);

	std::string generateFileListHtml(const std::string &directory);

	std::string loadErrorPage(int code);
	std::string replacePlaceHolders(std::string source,
									  const std::string &from,
									  const std::string &to);
	std::string getStatusText(int code);

	void GET(std::string &fsPath);
	void POST();
	void DELETE(const std::string &fsPath);
	bool handleLogin();
	bool handleSession();

  public:
	char *data;
	HttpRequest request;
	HttpResponse response;

	HTTP(const std::string &rawRequest, const ServerConfig &serverConfig);
	void parseRequest();
	void generateResponse();

	~HTTP();
};
