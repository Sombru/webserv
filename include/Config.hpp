#pragma once

#include "Webserv.hpp"

struct LocationConfig
{
	std::string path;
	std::string root;
	std::string alias;
	std::string index;
	std::string return_path;
	bool autoindex;
	std::vector<std::string> allow_methods;

	// For CGI
	std::vector<std::string> cgi_path;
	std::vector<std::string> cgi_ext;
};

struct ServerConfig
{
	std::string host;
	int port;
	std::string server_name;
	std::string error_page_404;
	size_t client_max_body_size;
	std::string root;
	std::string index;

	std::vector<LocationConfig> locations;
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

std::vector<Token> tokenize_config(const std::string &input);
std::ostream &operator<<(std::ostream &os, const Token &token);
std::ostream &operator<<(std::ostream &os, const std::vector<Token> &tokens);
std::ostream &operator<<(std::ostream &os, const ServerConfig &config);
std::ostream &operator<<(std::ostream &os, const LocationConfig &config);

void parse_server(ServerConfig &srv, const std::vector<Token> &tokens);