#pragma once

#include "Webserv.hpp"

#define DEFAULT_HOST "127.0.0.1"

struct LocationConfig
{
	std::string name;						// name of loc mandatory
	std::string root;						// path to root of its folder, defaults to root of the server
	std::string index;						// index a location, defaults to index of the server
	std::string return_path;				// redirection path
	std::string upload_dir;					// dir for POST request might be usefull
	bool autoindex;							// directory listing
	std::vector<std::string> allow_methods; // GET is always enabled

	// For CGI
	std::vector<std::string> cgi_path; // path to cgi scripts interpreters
	std::vector<std::string> cgi_ext;  // allowed extenstions
};

struct ServerConfig
{
	std::string host;			 	 // defaults to 127.0.0.1
	int port;					 	 // socket port to listen to, mandatory
	std::string server_name;	 	 // mandatory
	std::string root;  				 // root of this server mandatory
	size_t client_max_body_size; 	 // max recv size, mandatory
	std::string error_pages_dir;     // mandatory
	ssize_t default_location_index;  // mandatory

	std::vector<LocationConfig> locations; // locations for URI starting with location.name
};

enum TokenType
{
	WORD,
	LBRACE,
	RBRACE,
	SEMICOLON
};

struct Token
{
	TokenType type;
	std::string value;
};

Token addToken(TokenType type, std::string value);
std::vector<Token> tokenize_config(const std::string &input);
std::ostream &operator<<(std::ostream &os, const Token &token);
std::ostream &operator<<(std::ostream &os, const std::vector<Token> &tokens);
std::ostream &operator<<(std::ostream &os, const ServerConfig &config);
std::ostream &operator<<(std::ostream &os, const LocationConfig &config);

void parse_server(ServerConfig &srv, const std::vector<Token> &tokens, size_t &i);