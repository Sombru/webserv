#include "Config.hpp"

Token addToken(TokenType type, std::string value)
{
	Token token;

	token.type = type;
	token.value = value;
	return token;
}

std::vector<Token> tokenize_config(const std::string &input)
{
	std::vector<Token> tokens;
	std::string current;
	char c;
	for (size_t i = 0; i < input.length(); ++i)
	{
		c = input[i];
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
			while (input[i] != '\n')
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

