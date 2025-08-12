#pragma once

#include <string>
#include <vector>
#include <map>

class TokenIterator; // Forward declaration

struct LocationConfig
{
	std::string path;						 // uri of loc
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
	std::string name;							  // mandatory
	std::string index;							  // file to serve when server page requested default is index.html
	std::string host;							  // defaults to 127.0.0.1:8080, mam
	std::string root;							  // root of this server mandatory
	size_t clientMaxBodySize;					  // max recv size, mandatory
	std::string errorPage;						  // mandatory
	int timeout;								  // timeout before closing connection
	int maxEvents;								  // max number of events to handle in one request
	std::vector<LocationConfig> locations;		  // locations for URI starting with location.name
	std::map<std::string, std::string> mimeTypes; // mime types for this server
};

enum TokenType
{
	WORD,	  // all
	LBRACE,	  // {
	RBRACE,	  // }
	SEMICOLON // ;
};

struct Token
{
	TokenType type;
	std::string value;
};

#define MAND "___MANDATORY___"
#define DEFAULT "___DEFAULT___"
#define DEFAULT_INDEX "index.html"
#define DEFAULT_MAX_EVENTS 1000;

class Config
{
private:
	const char *configPath;			// path to config file
	std::string fileBuff;			// contents of a config file
	std::vector<Token> tokens;		// tokens of config file
	std::vector<ServerConfig> conf; // parsed config structure
	size_t currentTokenIndex;		// current position in tokens

	int error;
	std::string errorMsg;

	ServerConfig serverBase;
	LocationConfig LocationBase;

	// TokenIterator-based methods
	int parseServerConfig(TokenIterator &iter);
	int parseLocation(ServerConfig &server, TokenIterator &iter);
	bool parseSimpleDirective(TokenIterator &iter, std::string &result);
	bool parseAllowedMethods(LocationConfig &location, TokenIterator &iter);
	bool parseCgiDirective(LocationConfig &location, TokenIterator &iter);
	int parseTypesBlock(ServerConfig &server, TokenIterator &iter);

	void tokenize();
	int parseServerConfig(size_t &i);

public:
	Config(char *src);
	~Config();

	int parseConfig();
	int validateConfig();
	std::vector<ServerConfig> getConf() const;
};
