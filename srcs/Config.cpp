#include "Config.hpp"
#include "TokenIterator.hpp"
#include "Logger.hpp"
#include "Utils.hpp"

Config::Config(char *src)
	: configPath(src), currentTokenIndex(0), error(0)
{
	this->serverBase.name = MAND;
	this->serverBase.host = MAND;
	this->serverBase.index = DEFAULT;
	this->serverBase.root = MAND;
	this->serverBase.clientMaxBodySize = 0;
	this->serverBase.errorPage = MAND;
	this->serverBase.mimeTypes["text/plain"] = "plain";

	this->LocationBase.path = MAND;
	this->LocationBase.root = DEFAULT;
	this->LocationBase.alias = DEFAULT;
	this->LocationBase.index = DEFAULT;
	this->LocationBase.autoindex = false;
	this->LocationBase.allowedMethods.push_back("GET");
}

Config::~Config() {}

std::vector<ServerConfig> Config::getConf() const
{
	return this->conf;
}

static inline Token addToken(TokenType type, std::string value)
{
	Token token;

	token.type = type;
	token.value = value;
	return token;
}

void Config::tokenize()
{
	std::string current;
	char c;
	for (size_t i = 0; i < fileBuff.length(); ++i)
	{
		c = fileBuff[i];
		if (isspace(c))
		{
			if (!current.empty())
			{
				this->tokens.push_back(addToken(WORD, current));
				current.clear();
			}
		}
		else if (c == '{')
		{
			if (!current.empty())
			{
				this->tokens.push_back(addToken(WORD, current));
				current.clear();
			}
			this->tokens.push_back(addToken(LBRACE, "{"));
		}
		else if (c == '}')
		{
			if (!current.empty())
			{
				this->tokens.push_back(addToken(WORD, current));
				current.clear();
			}
			this->tokens.push_back(addToken(RBRACE, "}"));
		}
		else if (c == ';')
		{
			if (!current.empty())
			{
				this->tokens.push_back(addToken(WORD, current));
				current.clear();
			}
			this->tokens.push_back(addToken(SEMICOLON, ";"));
		}
		else if (c == '#')
		{
			while (fileBuff[i] && fileBuff[i] != '\n')
			{
				++i;
			}
		}
		else
			current += c;
	}
	if (!current.empty())
	{
		this->tokens.push_back(addToken(WORD, current));
	}
}

int Config::parseConfig()
{
	this->fileBuff = readFile(this->configPath);
	if (fileBuff == BADFILE)
	{
		ERROR("Could not open provided configuration or is empty");
		return -1;
	}

	tokenize();
	TokenIterator iter(this->tokens);

	while (iter.hasNext())
	{
		if (iter.currentType() != WORD)
		{
			WARNING("Expected directive name");
			iter.advance();
			continue;
		}

		if (iter.currentValue() == "server")
		{
			if (parseServerConfig(iter) == -1)
			{
				ERROR("Failed to parse server block");
				iter.skipToClosingBrace();
				if (iter.hasNext())
					iter.advance(); // skip the closing brace
			}
		}
		else
		{
			WARNING("Unknown top-level directive: " + iter.currentValue());
			iter.skipToNextDirective();
		}
	}

	return iter.hasErrors() ? -1 : 0;
}

// TokenIterator-based parsing methods
int Config::parseServerConfig(TokenIterator &iter)
{
	ServerConfig server = this->serverBase;
	std::set<std::string> seenDirectives; // Track seen directives

	// Parse: server <name> {
	if (!iter.consumeSpecificWord("server"))
		return -1;

	server.name = iter.consumeWord();
	if (server.name.empty())
		return -1;

	if (!iter.expectAndConsume(LBRACE))
		return -1;

	// Parse server block content
	while (iter.hasNext() && iter.currentType() != RBRACE)
	{
		if (iter.currentType() != WORD)
		{
			WARNING("Expected directive name but found " + iter.currentValue());
			iter.advance();
			continue;
		}

		std::string directive = iter.currentValue();

		// Check for redefinition (except location which can appear multiple times)
		if (directive != "location" && seenDirectives.find(directive) != seenDirectives.end())
		{
			WARNING("Redefinition of directive '" + directive + "' to " + iter.peekValue(1));
		}
		seenDirectives.insert(directive);

		if (directive == "location")
		{
			
			if (parseLocation(server, iter) == -1)
			{
				iter.skipToNextDirective(); // Error recovery
				continue;
			}
		}
		else if (directive == "host")
		{
			if (!parseSimpleDirective(iter, server.host))
				iter.skipToNextDirective();
		}
		else if (directive == "root")
		{
			if (!parseSimpleDirective(iter, server.root))
				iter.skipToNextDirective();
		}
		else if (directive == "index")
		{
			if (!parseSimpleDirective(iter, server.index))
				iter.skipToNextDirective();
		}
		else if (directive == "client_max_body_size")
		{
			std::string value;
			if (parseSimpleDirective(iter, value))
				server.clientMaxBodySize = std::atol(value.c_str());
			else
				iter.skipToNextDirective();
		}
		else if (directive == "error_page")
		{
			if (!parseSimpleDirective(iter, server.errorPage))
				iter.skipToNextDirective();
		}
		else if (directive == "types")
		{
			if (parseTypesBlock(server, iter) == -1)
				iter.skipToClosingBrace();
		}
		else
		{
			WARNING("Unknown server directive: " + directive);
			iter.skipToNextDirective();
		}
	}

	if (!iter.expectAndConsume(RBRACE))
		return -1;

	this->conf.push_back(server);
	return 0;
}

// Simple directive parser (directive value;)
bool Config::parseSimpleDirective(TokenIterator &iter, std::string &result)
{
	iter.advance(); // consume directive name

	if (!iter.expect(WORD))
		return false;

	result = iter.consumeWord();

	if (!iter.expectAndConsume(SEMICOLON))
		return false;

	return true;
}

// Location parsing with redefinition checking
int Config::parseLocation(ServerConfig &server, TokenIterator &iter)
{
	LocationConfig location = this->LocationBase;
	std::set<std::string> seenLocationDirectives; // Track seen location directives

	// Parse: location <path> {
	iter.advance(); // consume "location"

	location.path = iter.consumeWord();
	if (location.path.empty())
		return -1;

	if (!iter.expectAndConsume(LBRACE))
		return -1;

	// Parse location directives
	while (iter.hasNext() && iter.currentType() != RBRACE)
	{
		if (iter.currentType() != WORD)
		{
			WARNING("Expected location directive");
			iter.advance();
			continue;
		}

		std::string directive = iter.currentValue();

		// Check for redefinition in location block
		if (seenLocationDirectives.find(directive) != seenLocationDirectives.end())
		{
			WARNING("Redefinition of directive '" + directive + "' in location '" + location.path + "'");
		}
		seenLocationDirectives.insert(directive);

		// Special validation: root and alias are mutually exclusive
		if (directive == "root" && seenLocationDirectives.find("alias") != seenLocationDirectives.end())
		{
			WARNING("Location '" + location.path + "' has both 'root' and 'alias' directives - this may cause conflicts");
		}
		if (directive == "alias" && seenLocationDirectives.find("root") != seenLocationDirectives.end())
		{
			WARNING("Location '" + location.path + "' has both 'root' and 'alias' directives - this may cause conflicts");
		}

		if (directive == "root")
		{
			if (!parseSimpleDirective(iter, location.root))
				iter.skipToNextDirective();
		}
		else if (directive == "alias")
		{
			if (!parseSimpleDirective(iter, location.alias))
				iter.skipToNextDirective();
		}
		else if (directive == "index")
		{
			if (!parseSimpleDirective(iter, location.index))
				iter.skipToNextDirective();
		}
		else if (directive == "autoindex")
		{
			std::string value;
			if (parseSimpleDirective(iter, value))
				location.autoindex = (value == "on" || value == "true");
			else
				iter.skipToNextDirective();
		}
		else if (directive == "allowed_methods")
		{
			if (!parseAllowedMethods(location, iter))
				iter.skipToNextDirective();
		}
		else if (directive == "cgi")
		{
			if (!parseCgiDirective(location, iter))
				iter.skipToNextDirective();
		}
		else if (directive == "return")
		{
			if (!parseSimpleDirective(iter, location.returnPath))
				iter.skipToNextDirective();
		}
		else
		{
			WARNING("Unknown location directive: " + directive);
			iter.skipToNextDirective();
		}
	}

	if (!iter.expectAndConsume(RBRACE))
		return -1;

	server.locations.push_back(location);
	return 0;
}

// Parse allowed methods (can have multiple values)
bool Config::parseAllowedMethods(LocationConfig &location, TokenIterator &iter)
{
	iter.advance(); // consume "allowed_methods"

	location.allowedMethods.clear();

	// Collect all method names until semicolon
	while (iter.hasNext() && iter.currentType() != SEMICOLON)
	{
		if (iter.currentType() == WORD)
		{
			location.allowedMethods.push_back(iter.currentValue());
		}
		iter.advance();
	}

	if (!iter.expectAndConsume(SEMICOLON))
		return false;

	return true;
}

// Parse CGI directive (interpreter extension pairs)
bool Config::parseCgiDirective(LocationConfig &location, TokenIterator &iter)
{
	iter.advance(); // consume "cgi"

	// Parse: interpreter extension interpreter extension ... ;
	while (iter.hasNext() && iter.currentType() != SEMICOLON)
	{
		if (iter.currentType() == WORD && iter.peekType(1) == WORD)
		{
			std::string interpreter = iter.consumeWord();
			std::string extension = iter.consumeWord();
			location.cgi[extension] = interpreter;
		}
		else
		{
			iter.advance(); // skip unexpected token
		}
	}

	return iter.expectAndConsume(SEMICOLON);
}

// Parse types block
int Config::parseTypesBlock(ServerConfig &server, TokenIterator &iter)
{
	iter.advance(); // consume "types"

	if (!iter.expectAndConsume(LBRACE))
		return -1;

	while (iter.hasNext() && iter.currentType() != RBRACE)
	{
		if (iter.currentType() == WORD && iter.peekType(1) == WORD)
		{
			std::string mimeType = iter.consumeWord();
			std::string extension = iter.consumeWord();

			if (!iter.expectAndConsume(SEMICOLON))
			{
				iter.skipToNextDirective();
				continue;
			}

			server.mimeTypes[mimeType] = extension;
		}
		else
		{
			WARNING("Invalid types entry");
			iter.skipToNextDirective();
		}
	}

	return iter.expectAndConsume(RBRACE) ? 0 : -1;
}

int Config::validateConfig()
{
	for (size_t i = 0; i < this->conf.size(); ++i)
	{
		ServerConfig &server = this->conf[i];

		// Check mandatory fields
		if (server.name == MAND)
		{
			ERROR("Server name is mandatory");
			return -1;
		}
		if (server.host == MAND)
		{
			ERROR("Server host is mandatory");
			return -1;
		}
		if (server.root == MAND)
		{
			ERROR("Server root is mandatory");
			return -1;
		}
		if (server.errorPage == MAND)
		{
			ERROR("Server error_page is mandatory");
			return -1;
		}
		if (server.index == DEFAULT)
		{
			WARNING("No index for '" + server.name + "' defaults to 'index.html'");
			server.index = DEFAULT_INDEX;
		}
		for (size_t i = 0; i < server.locations.size(); ++i)
		{
			if (server.locations[i].root == DEFAULT && server.locations[i].alias == DEFAULT)
			{
				WARNING("No root and alias for location '" + server.locations[i].path + "' defaults to server root");
				server.locations[i].root = server.root;
			}
			if (server.locations[i].index == DEFAULT)
			{
				WARNING("No index for '" + server.locations[i].path + "' defaults to 'index.html'");
				server.locations[i].index = DEFAULT_INDEX;
			}
		}
	}
	return 0;
}
