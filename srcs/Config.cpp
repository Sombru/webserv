#include "Config.hpp"
#include "Logger.hpp"
#include "Utils.hpp"

#define MAND "___MANDATORY___"
#define DEFAULT "___DEFAULT___"

Config::Config(char *src)
: configPath(src)
{
	this->serverBase.name = MAND;	
	this->serverBase.host = MAND;
	this->serverBase.index = "index.html";
	this->serverBase.root = MAND;
	this->serverBase.clientMaxBodySize = 0;
	this->serverBase.errorPath = MAND;
	// this->serverBase.locations = NULL;
	this->serverBase.mimeTypes["text/plain"] = "plain"; 

	this->LocationBase.path = MAND;
	this->LocationBase.root = DEFAULT;
	// this->LocationBase.alias;
	this->LocationBase.index = "index.html";
	// this->LocationBase.index;
	this->LocationBase.autoindex = false;
	this->LocationBase.allowedMethods.push_back("GET");
	// this->LocationBase.cgi;
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
				i++;
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
	if (fileBuff == "BAD")
	{
		ERROR("Could not opena provided configuratin or is empty");
		return -1;
	}
	tokenize();

	for (size_t i = 0; i < this->tokens.size(); ++i)
	{
		if (parseServerConfig(i) == -1)
			return -1;
	}
	return EXIT_SUCCESS;
}

int parseConfigUnexpectedToken(const std::string &value)
{
	
}

int Config::parseServerConfig(size_t &i)
{

	while (i < this->tokens.size())
	{
		if (this->tokens[i].value != "server")
		{
			ERROR("Invalid directive `" + tokens[i].value + "'");
			return -1;
		}
		i++;
		if (tokens[i].type != WORD)
		{
			ERROR("Expected <name> after `server'");
			return -1;
		}
		i++;
		if (tokens[i].type != LBRACE)
		{
			ERROR("Expected `{' after server name");
		}
	}
	
}