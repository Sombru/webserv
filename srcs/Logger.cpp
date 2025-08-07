#include "Logger.hpp"
#include "Config.hpp"
#include "Webserv.hpp"

std::ostream &operator<<(std::ostream &os, const Token &token)
{
	switch (token.type)
	{
	case WORD:
		os << "WORD";
		break;
	case LBRACE:
		os << "LBRACE";
		break;
	case RBRACE:
		os << "RBRACE";
		break;
	case SEMICOLON:
		os << "SEMICOLON";
		break;
	default:
		os << "UNKNOWN";
		break;
	}
	os << "('" << token.value << "')\n";
	return os;
}

std::ostream &operator<<(std::ostream &os, const std::vector<Token> &tokens)
{
	os << "\n[";
	for (size_t i = 0; i < tokens.size(); ++i)
	{
		os << tokens[i];
	}
	os << "]\n";
	return os;
}
