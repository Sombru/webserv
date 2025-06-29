#include "Webserv.hpp"
#include "Config.hpp"
#include "ParserManager.hpp"

int ParserManager::parseConfig(const std::string &path)
{
	std::ifstream file(path.c_str());
	std::stringstream buffer;
	buffer << file.rdbuf();

	this->tokens = tokenize_config(buffer.str());
	parse_server(config, this->tokens);
	return 1;
}

size_t expect_word(const std::vector<Token> &tokens, size_t &i, const std::string &error_msg)
{
	if (i >= tokens.size() || tokens[i].type != WORD)
		throw std::runtime_error(error_msg);
	return i++;
}

void parse_location(LocationConfig &loc, const std::vector<Token> &tokens, size_t &i)
{
	loc.path = tokens[i - 1].value;
	if (tokens[i++].type != LBRACE)
		throw std::runtime_error("Expected '{' after location path");

	while (tokens[i].type != RBRACE)
	{
		std::string key = tokens[expect_word(tokens, i, "Expected directive key")].value;

		if (key == "root")
			loc.root = tokens[expect_word(tokens, i, "Expected root value")].value;
		else if (key == "alias")
			loc.alias = tokens[expect_word(tokens, i, "Expected alias value")].value;
		else if (key == "index")
			loc.index = tokens[expect_word(tokens, i, "Expected index value")].value;
		else if (key == "return")
			loc.return_path = tokens[expect_word(tokens, i, "Expected return value")].value;
		else if (key == "autoindex")
			loc.autoindex = tokens[expect_word(tokens, i, "Expected autoindex value")].value == "on";
		else if (key == "allow_methods")
			while (tokens[i].type == WORD)
				loc.allow_methods.push_back(tokens[i++].value);
		else if (key == "cgi_path")
			while (tokens[i].type == WORD)
				loc.cgi_path.push_back(tokens[i++].value);
		else if (key == "cgi_ext")
			while (tokens[i].type == WORD)
				loc.cgi_ext.push_back(tokens[i++].value);
		else
			throw std::runtime_error("Unknown location directive: " + key);

		if (tokens[i++].type != SEMICOLON)
			throw std::runtime_error("Missing semicolon in location block");
	}
	++i;
}

void parse_server(ServerConfig &srv, const std::vector<Token> &tokens)
{
	size_t i = 1;
	if (tokens[i++].type != LBRACE)
		throw std::runtime_error("Expected '{' after server");

	while (tokens[i].type != RBRACE)
	{
		std::string key = tokens[expect_word(tokens, i, "Expected directive key")].value;

		if (key == "listen")
			srv.port = std::atoi(tokens[expect_word(tokens, i, "Expected listen port")].value.c_str());
		else if (key == "host")
			srv.host = tokens[expect_word(tokens, i, "Expected host value")].value;
		else if (key == "server_name")
			srv.server_name = tokens[expect_word(tokens, i, "Expected server_name")].value;
		else if (key == "error_page")
			srv.error_page_404 = tokens[expect_word(tokens, i, "Expected error page")].value;
		else if (key == "client_max_body_size")
			srv.client_max_body_size = std::atoi(tokens[expect_word(tokens, i, "Expected size")].value.c_str());
		else if (key == "root")
			srv.root = tokens[expect_word(tokens, i, "Expected root value")].value;
		else if (key == "index")
			srv.index = tokens[expect_word(tokens, i, "Expected index value")].value;
		else if (key == "location")
		{
			LocationConfig loc;
			loc.path = tokens[expect_word(tokens, i, "Expected location path")].value;
			parse_location(loc, tokens, i);
			srv.locations.push_back(loc);
		}
		else
		{
			throw std::runtime_error("Unknown server directive: " + key);
		}

		if (tokens[i].type == SEMICOLON)
			++i;
	}

}
