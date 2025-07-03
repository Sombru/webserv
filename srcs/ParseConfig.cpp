#include "Webserv.hpp"
#include "Config.hpp"

size_t expect_word(const std::vector<Token> &tokens, size_t &i, const std::string &error_msg)
{
	if (i >= tokens.size() || tokens[i].type != WORD)
		throw std::runtime_error(error_msg);
	return i++;
}

void parse_location(LocationConfig &location, const std::vector<Token> &tokens, size_t &i)
{
	location.path = tokens[i - 1].value; // previous token was the location path

	if (tokens[i++].type != LBRACE)
		throw std::runtime_error("Expected '{' after location path");

	while (tokens[i].type != RBRACE)
	{
		std::string key = tokens[expect_word(tokens, i, "Expected directive key")].value;

		if (key == "root")
			location.root = tokens[expect_word(tokens, i, "Expected root value")].value;
		else if (key == "alias")
			location.alias = tokens[expect_word(tokens, i, "Expected alias value")].value;
		else if (key == "index")
			location.index = tokens[expect_word(tokens, i, "Expected index value")].value;
		else if (key == "return")
			location.return_path = tokens[expect_word(tokens, i, "Expected return value")].value;
		else if (key == "autoindex")
			location.autoindex = tokens[expect_word(tokens, i, "Expected autoindex value")].value == "on";
		else if (key == "allow_methods")
			while (tokens[i].type == WORD)
				location.allow_methods.push_back(tokens[i++].value);
		else if (key == "cgi_path")
			while (tokens[i].type == WORD)
				location.cgi_path.push_back(tokens[i++].value);
		else if (key == "cgi_ext")
			while (tokens[i].type == WORD)
				location.cgi_ext.push_back(tokens[i++].value);
		else
			throw std::runtime_error("Unknown location directive: " + key);

		if (tokens[i++].type != SEMICOLON)
			throw std::runtime_error("Missing semicolon in location block");
	}
	++i;
}

void parse_server(ServerConfig &server, const std::vector<Token> &tokens, size_t &i)
{
	if (tokens[i++].type != LBRACE)
		throw std::runtime_error("Expected '{' after server");

	while (tokens[i].type != RBRACE)
	{
		std::string key = tokens[expect_word(tokens, i, "Expected directive key")].value;

		if (key == "listen" || key == "port")
			server.port = std::atoi(tokens[expect_word(tokens, i, "Expected listen port")].value.c_str());
		else if (key == "host")
			server.host = tokens[expect_word(tokens, i, "Expected host value")].value;
		else if (key == "server_name")
			server.server_name = tokens[expect_word(tokens, i, "Expected server_name")].value;
		else if (key == "error_pages")
			server.error_pages_dir = tokens[expect_word(tokens, i, "Expected error page")].value;
		else if (key == "client_max_body_size")
			server.client_max_body_size = std::atoi(tokens[expect_word(tokens, i, "Expected size")].value.c_str());
		else if (key == "root")
			server.root = tokens[expect_word(tokens, i, "Expected root value")].value;
		else if (key == "index")
			server.index = tokens[expect_word(tokens, i, "Expected index value")].value;
		else if (key == "location")
		{
			LocationConfig location;
			location.path = tokens[expect_word(tokens, i, "Expected location path")].value;
			parse_location(location, tokens, i);
			server.locations.push_back(location);
		}
		else
		{
			throw std::runtime_error("Unknown server directive: " + key);
		}
		if (tokens[i].type == SEMICOLON)
			++i;
	}
	++i;
}

