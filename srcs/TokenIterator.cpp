#include "Config.hpp"
#include "TokenIterator.hpp"
#include "Webserv.hpp"

TokenIterator::TokenIterator(const std::vector<Token> &tokensSrc)
	: tokens(tokensSrc), position(0), hasError(false)
{
}

TokenIterator::~TokenIterator() {}

bool TokenIterator::hasNext() const
{
	return position < tokens.size();
}

bool TokenIterator::isAtEnd() const
{
	return position >= tokens.size();
}

size_t TokenIterator::getPosition() const
{
	return position;
}

void TokenIterator::setPosition(size_t pos)
{
	if (pos <= tokens.size())
		position = pos;
	else
		setError("Attempted to set position beyond token vector bounds");
}

// Error handling
bool TokenIterator::hasErrors() const 
{ 
	return hasError; 
}
const std::string &TokenIterator::getLastError() const 
{
	return lastError; 
}

const Token &TokenIterator::current() const
{
	static Token invalidToken = {WORD, "__INVALID__"};
	if (isAtEnd())
	{
		const_cast<TokenIterator *>(this)->setError("Attempting to acces tokens beyon end");
		return invalidToken;
	}
	return tokens[position];
}

TokenType TokenIterator::currentType() const
{
	return isAtEnd() ? WORD : tokens[position].type;
}

const std::string &TokenIterator::currentValue() const
{
	static std::string empty = "";
	return isAtEnd() ? empty : tokens[position].value;
}

void TokenIterator::advance()
{
	if (!isAtEnd())
		++position;
	else
		setError("Cannot advance beyond end of tokens");
}

void TokenIterator::retreat()
{
	if (position > 0)
		--position;
	else
		setError("Cannot retreat before begining of tokens");
}

bool TokenIterator::expect(TokenType type)
{
	if (isAtEnd())
	{
		setError("Expected " + tokenTypeToString(type) + " but reached the end of file");
		return false;
	}

	if (currentType() != type)
	{
		setError("Expected " + tokenTypeToString(type) +
				 " but found " + tokenTypeToString(currentType()) +
				 " ('" + currentValue() + "')");
		return false;
	}
	return true;
}

bool TokenIterator::expectAndConsume(TokenType expectedType)
{
	if (!expect(expectedType))
		return false;
	advance();
	return true;
}

// Consume a word token and return its value
std::string TokenIterator::consumeWord()
{
	if (!expect(WORD))
		return "";

	std::string value = currentValue();
	advance();
	return value;
}

// Consume a specific word value
bool TokenIterator::consumeSpecificWord(const std::string &expectedWord)
{
	if (!expect(WORD))
		return false;

	if (currentValue() != expectedWord)
	{
		setError("Expected word '" + expectedWord + "' but found '" + currentValue() + "'");
		return false;
	}

	advance();
	return true;
}

// Skip to next significant token (useful for error recovery)
void TokenIterator::skipToNextDirective()
{
	while (hasNext() && currentType() != SEMICOLON && currentType() != RBRACE)
		advance();
	if (currentType() == SEMICOLON)
		advance();
}

void TokenIterator::skipToClosingBrace()
{
	int braceCount = 0;
	while (hasNext())
	{
		if (currentType() == LBRACE)
			braceCount++;
		else if (currentType() == RBRACE)
		{
			if (braceCount == 0)
				break;
			braceCount--;
		}
		advance();
	}
}

void TokenIterator::clearError()
{
	hasError = false;
	lastError.clear();
}

void TokenIterator::setError(const std::string &message)
{
	hasError = true;
	lastError = message;
	ERROR("Token parsing error at position " + intToString(position) + ": " + message);
}

// Utility methods
std::string TokenIterator::tokenTypeToString(TokenType type) const
{
	switch (type)
	{
	case WORD:
		return "WORD";
	case LBRACE:
		return "LBRACE '{'";
	case RBRACE:
		return "RBRACE '}'";
	case SEMICOLON:
		return "SEMICOLON ';'";
	default:
		return "UNKNOWN";
	}
}

// Peek operations (look ahead without consuming)
TokenType TokenIterator::peekType(size_t offset = 1) const
{
	size_t peekPos = position + offset;
	return (peekPos < tokens.size()) ? tokens[peekPos].type : WORD;
}

std::string TokenIterator::peekValue(size_t offset = 1) const
{
	size_t peekPos = position + offset;
	return (peekPos < tokens.size()) ? tokens[peekPos].value : "";
}

// Check if current directive is complete (ends with semicolon)
bool TokenIterator::isDirectiveComplete() const
{
	for (size_t i = position; i < tokens.size(); ++i)
	{
		if (tokens[i].type == SEMICOLON)
			return true;
		if (tokens[i].type == RBRACE || tokens[i].type == LBRACE)
			return false;
	}
	return false;
}

// Count tokens until delimiter
size_t TokenIterator::countTokensUntil(TokenType delimiter) const
{
	size_t count = 0;
	for (size_t i = position; i < tokens.size(); ++i)
	{
		if (tokens[i].type == delimiter)
			break;
		count++;
	}
	return count;
}