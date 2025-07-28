#include "Webserv.hpp"
#include "Config.hpp"
#include "Utils.hpp"
#include "Logger.hpp"

Config::Config() {}

Config::~Config() {}

static Token addToken(TokenType type, std::string value)
{
	Token token;

	token.type = type;
	token.value = value;
	return token;
}

/// @brief parses contens of a file into tokens of TYPE and VALUE, see Token
/// @param fileBuff file contents(cant be empty)
/// @return // vector of tokens read from file
std::vector<Token> Config::tokenize(const std::string &fileBuff)
{
	std::vector<Token> tokens;
	std::string current;
	char c;
	for (size_t i = 0; i < fileBuff.length(); ++i)
	{
		c = fileBuff[i];
		if (isspace(c))
		{
			if (!current.empty())
			{
				tokens.push_back(addToken(WORD, current));
				current.clear();
			}
		}
		else if (c == '{')
		{
			if (!current.empty())
			{
				tokens.push_back(addToken(WORD, current));
				current.clear();
			}
			tokens.push_back(addToken(LBRACE, "{"));
		}
		else if (c == '}')
		{
			if (!current.empty())
			{
				tokens.push_back(addToken(WORD, current));
				current.clear();
			}
			tokens.push_back(addToken(RBRACE, "}"));
		}
		else if (c == ';')
		{
			if (!current.empty())
			{
				tokens.push_back(addToken(WORD, current));
				current.clear();
			}
			tokens.push_back(addToken(SEMICOLON, ";"));
		}
		else if (c == '#')
		{
			while (fileBuff[i] && fileBuff[i] != '\n')
			{
				i++;
			}
		}
		else
			current += c;
	}
	if (!current.empty())
	{
		tokens.push_back(addToken(WORD, current));
	}
	return tokens;
}

/// @brief parses tokens into "server" blocks, see ServerConfig
// known directives "server"
/// @param tokens vector of tokens read from a config file
/// @return vector of "server" blocks from a file
std::vector<ServerConfig> Config::parseConfig(const char *path)
{
	std::string fileBuff = readFile(path);
	if (fileBuff == BADFILE)
		throw std::runtime_error("Could not open a file at given path");
	if (fileBuff == EMPTY)
		throw std::runtime_error("given config file is empty");
	
	std::vector<Token> tokens = Config::tokenize(fileBuff);
	Logger::info("Tokenization complete. Token count: " + intToString(tokens.size()));
	std::vector<ServerConfig> servers;
	size_t index = 0;

	while (index < tokens.size())
	{
		if (tokens[index].type == WORD && tokens[index].value == "server")
		{
			ServerConfig server = Config::parseServerConfig(tokens, index);
			servers.push_back(server);
		}
		else
		{
			// Skip unknown tokens or handle errors
			index++;
		}
	}
	Config::validateConfig(servers);
	Logger::info("Configuration validation passed");
	return servers;
}

/// @brief parses a single "server" block from config
// directives: "server", "host", "listen", "server_name", "root", "client_max_body_size", "location"
/// @param tokens tokens to parse
/// @param index index of current token to parse
/// @return a "server" block, see ServerConfig
ServerConfig Config::parseServerConfig(const std::vector<Token> &tokens, size_t &index)
{
	ServerConfig server;

	server.host = DEFAULT_HOST;
	server.port = -1;
	server.serverNname = "";
	server.root = "";
	server.clientMaxBodySize = 0;
	server.errorPath = "";
	server.defaultLocation = NULL;

	// Expect "server" token
	if (index >= tokens.size() || tokens[index].value != "server")
		throw std::runtime_error("Expected 'server' keyword");
	index++;

	// Expect opening brace
	if (index >= tokens.size() || tokens[index].type != LBRACE)
		throw std::runtime_error("Expected '{' after 'server'");
	index++;

	// Parse server directives
	while (index < tokens.size() && tokens[index].type != RBRACE)
	{
		if (tokens[index].type == WORD)
		{
			std::string directive = tokens[index].value;
			index++;

			if (directive == "host")
			{
				if (index >= tokens.size() || tokens[index].type != WORD)
					throw std::runtime_error("Expected value after 'host'");
				server.host = tokens[index].value;
				index++;
			}
			else if (directive == "listen")
			{
				if (index >= tokens.size() || tokens[index].type != WORD)
					throw std::runtime_error("Expected value after 'listen'");
				server.port = atoi(tokens[index].value.c_str());
				index++;
			}
			else if (directive == "server_name")
			{
				if (index >= tokens.size() || tokens[index].type != WORD)
					throw std::runtime_error("Expected value after 'server_name'");
				server.serverNname = tokens[index].value;
				index++;
			}
			else if (directive == "root")
			{
				if (index >= tokens.size() || tokens[index].type != WORD)
					throw std::runtime_error("Expected value after 'root'");
				server.root = tokens[index].value;
				index++;
			}
			else if (directive == "client_max_body_size")
			{
				if (index >= tokens.size() || tokens[index].type != WORD)
					throw std::runtime_error("Expected value after 'client_max_body_size'");
				server.clientMaxBodySize = atoi(tokens[index].value.c_str());
				index++;
			}
			else if (directive == "error_page")
			{
				if (index >= tokens.size() || tokens[index].type != WORD)
					throw std::runtime_error("Expected value after 'error_page'");
				server.errorPath = tokens[index].value;
				index++;
			}
			else if (directive == "location")
			{
				// Parse location block
				LocationConfig location = Config::parseLocation(tokens, index);
				server.locations.push_back(location);
				continue; // parseLocation handles its own indexing
			}

			// Expect semicolon after directive value
			if (index < tokens.size() && tokens[index].type == SEMICOLON)
				index++;
		}
		else
		{
			index++;
		}
	}

	// Expect closing brace
	if (index >= tokens.size() || tokens[index].type != RBRACE)
		throw std::runtime_error("Expected '}' to close server block");
	index++;

	return server;
}

/// @brief parses location block in ServerConfig
/// @param tokens 
/// @param index 
/// @return 
LocationConfig Config::parseLocation(const std::vector<Token> &tokens, size_t &index)
{
	LocationConfig location;

	// Initialize defaults
	location.name = "";
	location.root = "";
	location.index = "";
	location.returnPath = "";
	location.uploadDir = "";
	location.autoindex = false;

	// Expect location path
	if (index >= tokens.size() || tokens[index].type != WORD)
		throw std::runtime_error("Expected location name");
	location.name = tokens[index].value;
	index++;

	// Expect opening brace
	if (index >= tokens.size() || tokens[index].type != LBRACE)
		throw std::runtime_error("Expected '{' after location name");
	index++;

	// Parse location directives
	while (index < tokens.size() && tokens[index].type != RBRACE)
	{
		if (tokens[index].type == WORD)
		{
			std::string directive = tokens[index].value;
			index++;

			if (directive == "root")
			{
				if (index >= tokens.size() || tokens[index].type != WORD)
					throw std::runtime_error("Expected value after 'root'");
				location.root = tokens[index].value;
				index++;
			}
			else if (directive == "index")
			{
				if (index >= tokens.size() || tokens[index].type != WORD)
					throw std::runtime_error("Expected value after 'index'");
				location.index = tokens[index].value;
				index++;
			}
			else if (directive == "autoindex")
			{
				if (index >= tokens.size() || tokens[index].type != WORD)
					throw std::runtime_error("Expected value after 'autoindex'");
				location.autoindex = (tokens[index].value == "on");
				index++;
			}
			else if (directive == "allow_methods")
			{
				// Parse multiple methods until semicolon
				while (index < tokens.size() && tokens[index].type != SEMICOLON)
				{
					if (tokens[index].type == WORD)
						location.allowedMethods.push_back(tokens[index].value);
					index++;
				}
				continue; // Skip semicolon handling below
			}
			else if (directive == "cgi_ext")
			{
				// Parse CGI extensions
				std::vector<std::string> extensions;
				while (index < tokens.size() && tokens[index].type != SEMICOLON)
				{
					if (tokens[index].type == WORD)
						extensions.push_back(tokens[index].value);
					index++;
				}
				for (size_t i = 0; i < extensions.size(); i++)
					location.cgi[extensions[i]] = ""; 
				continue;
			}
			else if (directive == "cgi_path")
			{
				// Parse CGI paths and match with extensions
				std::vector<std::string> paths;
				while (index < tokens.size() && tokens[index].type != SEMICOLON)
				{
					if (tokens[index].type == WORD)
						paths.push_back(tokens[index].value);
					index++;
				}

				// Match paths with extensions (assumes same order)
				size_t pathIndex = 0;
				for (std::map<std::string, std::string>::iterator it = location.cgi.begin();
					 it != location.cgi.end() && pathIndex < paths.size(); ++it, ++pathIndex)
				{
					it->second = paths[pathIndex];
				}
				continue;
			}
			else if (directive == "return")
			{
				if (index >= tokens.size() || tokens[index].type != WORD)
					throw std::runtime_error("Expected value after 'return'");
				location.returnPath = tokens[index].value;
				index++;
			}
			else if (directive == "upload_dir")
			{
				if (index >= tokens.size() || tokens[index].type != WORD)
					throw std::runtime_error("Expected value after 'upload_dir'");
				location.uploadDir = tokens[index].value;
				index++;
			}

			// Expect semicolon after directive value
			if (index < tokens.size() && tokens[index].type == SEMICOLON)
				index++;
		}
		else
		{
			index++;
		}
	}

	// Expect closing brace
	if (index >= tokens.size() || tokens[index].type != RBRACE)
		throw std::runtime_error("Expected '}' to close location block");
	index++;

	return location;
}

void Config::validateConfig(std::vector<ServerConfig> &config)
{
	if (config.empty())
		throw std::runtime_error("Configuration validation failed: No server blocks found");

	for (size_t i = 0; i < config.size(); i++)
	{
		ServerConfig &server = config[i];
		
		// Validate mandatory server fields
		if (server.port == -1)
			throw std::runtime_error("Configuration validation failed: Server " + intToString(i + 1) + " missing mandatory 'listen' directive");
		
		if (server.serverNname.empty())
			throw std::runtime_error("Configuration validation failed: Server " + intToString(i + 1) + " missing mandatory 'server_name' directive");
		
		if (server.root.empty())
			throw std::runtime_error("Configuration validation failed: Server " + intToString(i + 1) + " missing mandatory 'root' directive");
		
		if (server.clientMaxBodySize == 0)
			throw std::runtime_error("Configuration validation failed: Server " + intToString(i + 1) + " missing mandatory 'client_max_body_size' directive");
		
		if (server.errorPath.empty())
			throw std::runtime_error("Configuration validation failed: Server " + intToString(i + 1) + " missing mandatory 'error_page' directive");
		
		// Check for duplicate ports
		for (size_t j = i + 1; j < config.size(); j++)
		{
			if (config[j].port == server.port && config[j].host == server.host)
				throw std::runtime_error("Configuration validation failed: Duplicate server configuration for " + server.host + ":" + intToString(server.port));
		}
		
		// Validate locations and find default location
		bool hasDefaultLocation = false;
		for (size_t j = 0; j < server.locations.size(); j++)
		{
			LocationConfig &location = server.locations[j];
			
			// Check if this is the default location "/"
			if (location.name == "/")
			{
				hasDefaultLocation = true;
				server.defaultLocation = &server.locations[j];
			}
			
			// Validate mandatory location fields
			if (location.name.empty())
				throw std::runtime_error("Configuration validation failed: Server " + intToString(i + 1) + " has location with empty name");
			
			// Set default values for location
			if (location.root.empty())
				location.root = server.root; // defaults to root of the server
			
			// Ensure GET is always enabled
			bool hasGet = false;
			for (size_t k = 0; k < location.allowedMethods.size(); k++)
			{
				if (location.allowedMethods[k] == "GET")
				{
					hasGet = true;
					break;
				}
			}
			if (!hasGet)
				location.allowedMethods.push_back("GET");
		}
		
		// Now set default index values after we have the defaultLocation pointer
		for (size_t j = 0; j < server.locations.size(); j++)
		{
			LocationConfig &location = server.locations[j];
			
			if (location.index.empty())
			{
				// Set default index if server has a default location with index
				if (server.defaultLocation && !server.defaultLocation->index.empty())
					location.index = server.defaultLocation->index;
				else
					location.index = "index.html"; // fallback default
			}
		}
		
		// Check for mandatory default location
		if (!hasDefaultLocation)
			throw std::runtime_error("Configuration validation failed: Server " + intToString(i + 1) + " missing mandatory default location '/'");
		
		// Validate port range
		if (server.port < 1 || server.port > 65535)
			throw std::runtime_error("Configuration validation failed: Server " + intToString(i + 1) + " port " + intToString(server.port) + " is out of valid range (1-65535)");
		
		// Validate client_max_body_size (reasonable limits)
		if (server.clientMaxBodySize > 1073741824) // 1GB limit
			throw std::runtime_error("Configuration validation failed: Server " + intToString(i + 1) + " client_max_body_size " + intToString(server.clientMaxBodySize) + " exceeds maximum allowed (1GB)");
	}
}