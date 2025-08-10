#pragma once

#include "Config.hpp"
#include "Logger.hpp"
#include "Utils.hpp"
#include <vector>

class TokenIterator
{
private:
	const std::vector<Token> &tokens;
	size_t position;
	bool hasError;
	std::string lastError;

public:
	TokenIterator(const std::vector<Token> &tokenVec);
	~TokenIterator();

	// Basic navigation
	bool hasNext() const;

	bool isAtEnd() const;
	size_t getPosition() const;
	void setPosition(size_t pos);

	// Current token access
	const Token &current() const;

	TokenType currentType() const;

	const std::string &currentValue() const;

	// Movement operations
	void advance();

	void retreat();
	// Expectation methods with better error messages
	bool expect(TokenType expectedType);

	bool expectAndConsume(TokenType expectedType);

	// Consume a word token and return its value
	std::string consumeWord();

	// Consume a specific word value
	bool consumeSpecificWord(const std::string &expectedWord);

	// Skip to next significant token (useful for error recovery)
	void skipToNextDirective();

	void skipToClosingBrace();

	// Error handling
	bool hasErrors() const;
	const std::string &getLastError() const;
	void clearError();

	void setError(const std::string &message);

	// Utility methods
	std::string tokenTypeToString(TokenType type) const;

	// Peek operations (look ahead without consuming)
	TokenType peekType(size_t offset) const;

	std::string peekValue(size_t offset) const;

	// Check if current directive is complete (ends with semicolon)
	bool isDirectiveComplete() const;

	// Count tokens until delimiter
	size_t countTokensUntil(TokenType delimiter) const;
};
