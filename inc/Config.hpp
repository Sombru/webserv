#pragma once

#include <string>
#include <vector>
#include <map>

struct LocationConfig
{
	std::string path;						 // path of to mandatory
	std::string root;						 // path to root of its folder, defaults to root of the server
	std::string alias;						 // alias to root of the its folder(doesnt append with path) 
	std::string index;						 // index a location
	std::string returnPath;					 // redirection path
	std::string uploadDir;					 // dir for POST request might be usefull
	bool autoindex;							 // directory listing
	std::vector<std::string> allowedMethods; // GET is always enabled

	// For CGI
	std::map<std::string, std::string> cgi; // path to cgi scripts interpreters + extenstion for it
};

struct ServerConfig
{
	std::string serverNname;			   // mandatory
	std::string host;					   // defaults to 127.0.0.1:8080, mam
	std::string root;					   // root of this server mandatory
	size_t clientMaxBodySize;			   // max recv size, mandatory
	std::string errorPath;				   // mandatory
	std::vector<LocationConfig> locations; // locations for URI starting with location.name
	std::map<std::string, std::string> mimeTypes; // mime types for this server
};

// default values for config files

// #define DEFAULT_HOST "127.0.0.1"
// #define DEFAULT_INDEX_HTML "index.html"
// #define DEFAULT_INDEX_PHP "index.php"
// #define DEFAULT_REQUEST_BODY_SIZE 0


enum TokenType
{
	WORD, // all
	LBRACE, // {
	RBRACE, // }
	SEMICOLON // ;
};

struct Token
{
	TokenType type;
	std::string value;
};

class Config
{
private:
	/* data */
public:
	static std::vector<Token> tokenize(std::string &fileBuff);
	static std::vector<ServerConfig> parseConfig(const char*);
	static ServerConfig parseServerConfig(const std::vector<Token> &tokens, size_t &index);
	static LocationConfig parseLocation(const std::vector<Token> &tokens, size_t &index);
	static void validateConfig(std::vector<ServerConfig> &config);

};
