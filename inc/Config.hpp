#pragma once

#include "Webserv.hpp"

#define DEFAULT_HOST "127.0.0.1"

struct LocationConfig
{
	std::string name;						 // name of loc mandatory
	std::string root;						 // path to root of its folder, defaults to root of the server
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
	std::string host;					   // defaults to 127.0.0.1
	int port;							   // socket port to listen to, mandatory
	std::string serverNname;			   // mandatory
	std::string root;					   // root of this server mandatory
	size_t clientMaxBodySize;			   // max recv size, mandatory
	std::string errorPath;				   // mandatory
	LocationConfig *defaultLocation;	   // for location named "/" mandatory
	std::vector<LocationConfig> locations; // locations for URI starting with location.name
};

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
	Config();

	static std::vector<Token> tokenize(const std::string &fileBuff);
	static std::vector<ServerConfig> parseConfig(const char*);
	static ServerConfig parseServerConfig(const std::vector<Token> &tokens, size_t &index);
	static LocationConfig parseLocation(const std::vector<Token> &tokens, size_t &index);
	static void validateConfig(std::vector<ServerConfig> &config);

	~Config();
};

// Token addToken(TokenType type, std::string value);
// std::vector<Token> tokenize_config(const std::string &input);
// std::ostream &operator<<(std::ostream &os, const Token &token);
// std::ostream &operator<<(std::ostream &os, const std::vector<Token> &tokens);
// std::ostream &operator<<(std::ostream &os, const ServerConfig &config);
// std::ostream &operator<<(std::ostream &os, const LocationConfig &config);

// void parse_server(ServerConfig &srv, const std::vector<Token> &tokens, size_t &i);