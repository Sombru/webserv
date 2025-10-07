#pragma once
#include "Logger.hpp"
#include "Webserv.hpp"

#define HTTP_VERSION "HTTP/1.1"

// some headers we use
#define CONTENT_TYPE "Content-Type"
#define CONTENT_LENGHT "Content-Lenght"
#define LOCATION "Location"
#define CONNECTION "Connection"
#define SET_COOKIE "Set-Cookie"
#define CONTENT_DISPOSITION "Content-Disposition"

struct HttpRequest
{
	std::string method;								 // e.g. GET
	std::string path;								 // e.g. /about.html
	std::string target_file;						 // e.g. about.html
	std::string query_string;						 // e.g ?alice=18
	std::string version;							 // e.g. "HTTP/1.1"
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
};

class HTTP
{
private:

	std::string chunkBuffer;

	std::string resolveRequestPath();
	std::string getMimeType(const std::string &path);

	std::map<std::string, std::string> parseQuery(const std::string &query_string);

	int methodAllowed(std::string &requestMethod);
	void redirect(const std::string &returnPath);

	void findBestLocation();
	void handleConnectionHeader();

	void buildResponse(int code, std::string &fsTarget);
	void buildErrorRespose(int code);
	void buildResponse(int code);

	void addHeaders(const std::string &header, const std::string &value);

	// Chunked transfer encoding methods
	void generateChunkAcceptedResponse(); // 202 Accepted response
	bool isChunkComplete(const std::string& data);
	std::string extractChunkSize(const std::string& data);
	std::string extractChunkData(const std::string& data, size_t chunkSize);

	std::string buildAutoIndexHTML(std::string &fsTarget);

	std::string loadErrorPage(int code);
	std::string replacePlaceHolders(std::string source,
									const std::string &from,
									const std::string &to);
	std::string getStatusText(int code);
	std::string readFileSmart(const std::string &fsPath);	

	void GET(std::string &fsPath);
	void POST();

	// CGI execution helper
	// Returns true if CGI was executed and response is filled
	bool executeCgi(const std::string &scriptPath, const std::string &interpreter, const std::string &requestBody);
	void DELETE(const std::string &fsPath);
	bool handleLogin();
	bool handleSession();

public:
	char *data;
	ServerConfig &serverConfig;
	HttpRequest request;
	HttpResponse response;

	bool processChunkData(const std::string& chunkData);
	HTTP(ServerConfig &config);
	HTTP(const HTTP &other);
	HTTP &operator=(const HTTP &other);
	void parseRequest(const std::string &rawRequest);
	void generateResponse();

	~HTTP();
};
