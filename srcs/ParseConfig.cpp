#include "Webserv.hpp"
#include "Config.hpp"
#include "Logger.hpp"

// inline bool validate_methods() 

void validate_server(ServerConfig& server)
{
	// Validate mandatory fields
	if (server.port <= 0 || server.port > 65535)
		throw std::runtime_error("Invalid port: port must be between 1 and 65535");
	
	if (server.server_name.empty())
		throw std::runtime_error("server_name is mandatory");
	
	if (server.root.empty())
		throw std::runtime_error("root is mandatory");
	
	if (server.index.empty())
		throw std::runtime_error("index is mandatory");
	
	if (server.client_max_body_size == 0)
		throw std::runtime_error("client_max_body_size is mandatory and must be > 0");
	
	if (server.error_pages_dir.empty())
		throw std::runtime_error("error_pages_dir is mandatory");
	if (server.host.empty())
	{
		Logger::info("host should be set (defaults to 127.0.0.1)");
		server.host = DEFAULT_HOST;
	}
	
	// Validate locations
	for (unsigned int i = 0; i < server.locations.size(); i++)
	{
		if (server.locations[i].name.empty())
			throw std::runtime_error("location name is mandatory");

		if (server.locations[i].allow_methods.empty())
			server.locations[i].allow_methods.push_back("GET");
		for (unsigned int k = 0; k < server.locations[i].allow_methods.size(); k++)
			if (server.locations[i].allow_methods[k] != "GET" &&
				server.locations[i].allow_methods[k] != "POST" &&
				server.locations[i].allow_methods[k] !=  "DELETE")
				throw std::runtime_error("unknownt HTTP method");
	}
}

size_t expect_word(const std::vector<Token> &tokens, size_t &i, const std::string &error_msg)
{
	if (i >= tokens.size() || tokens[i].type != WORD)
		throw std::runtime_error(error_msg);
	return i++;
}

void parse_location(LocationConfig &location, const std::vector<Token> &tokens, size_t &i)
{
	location.root = tokens[i - 1].value; // previous token was the location path

	if (tokens[i++].type != LBRACE)
		throw std::runtime_error("Expected '{' after location path");

	while (tokens[i].type != RBRACE)
	{
		std::string key = tokens[expect_word(tokens, i, "Expected directive key")].value;

		if (key == "root")
			location.root = tokens[expect_word(tokens, i, "Expected root value")].value;
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
		else if (key == "upload_dir")
			location.upload_dir = tokens[expect_word(tokens, i, "Expected upload dir value")].value;
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
			location.name  = tokens[expect_word(tokens, i, "Expected location name")].value;
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
	validate_server(server);
}

