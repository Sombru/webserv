#include "Config.hpp"
#include "Logger.hpp"
#include "Utils.hpp"

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
std::vector<Token> Config::tokenize(std::string &fileBuff)
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

ServerConfig Config::parseServerConfig(const std::vector<Token> &tokens, size_t &index)
{
	static const char *serverDirectives[] = {"types", "client_max_body_size",
	"host", "root", "index", "error_page", "location"};
	
}
