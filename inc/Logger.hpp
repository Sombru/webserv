// Logger.hpp
#pragma once

#define GREEN "\033[32m"		// Green
#define YELLOW "\033[33m"		// Yellow
#define RED "\033[31m"			// Red
#define CYAN "\033[36m"			// Cyan
#define BRIGHT_BLACK "\033[90m" // Bright Black (Gray)
#define RESET "\033[0m"			// Reset to default

#include <ctime>
#include <string>
#include <iostream>
#include "Config.hpp"

enum LogLevel
{
	INFO,
	WARNING,
	ERROR,
	DEBUG
};

std::ostream &operator<<(std::ostream &os, const Token &token);
std::ostream &operator<<(std::ostream &os, const std::vector<Token> &tokens);
std::ostream &operator<<(std::ostream &os, const Config &conf);
std::ostream &operator<<(std::ostream &os, const ServerConfig &server);
std::ostream &operator<<(std::ostream &os, const LocationConfig &location);

#define errstr std::string(strerror(errno))

#define LOGGER_INFO(msg) Logger::info(msg, __func__, __FILE__, __LINE__)
#define LOGGER_WARNING(msg) Logger::warning(msg, __func__, __FILE__, __LINE__)
#define LOGGER_ERROR(msg) Logger::error(msg, __func__, __FILE__, __LINE__)
#define LOGGER_DEBUG(msg) Logger::debug(msg, __func__, __FILE__, __LINE__)

#define INFO(msg) Logger::info(msg, __func__, __FILE__, __LINE__)
#define WARNING(msg) Logger::warning(msg, __func__, __FILE__, __LINE__)
#define ERROR(msg) Logger::error(msg, __func__, __FILE__, __LINE__)
#define DEBUG(msg) Logger::debug(msg, __func__, __FILE__, __LINE__)

class Logger
{
public:
	template <typename T>
	static void info(const T &message, const char *func, const char *file, int line)
	{
		log(::INFO, message, func, file, line);
	}

	template <typename T>
	static void warning(const T &message, const char *func, const char *file, int line)
	{
		log(::WARNING, message, func, file, line);
	}

	template <typename T>
	static void error(const T &message, const char *func, const char *file, int line)
	{
		log(::ERROR, message, func, file, line);
	}

	template <typename T>
	static void debug(const T &message, const char *func, const char *file, int line)
	{
		log(::DEBUG, message, func, file, line);
	}

private:
	template <typename T>
	static void log(LogLevel level, const T &message, const char *func, const char *file, int line)
	{
		std::string levelStr;
		switch (level)
		{
		case ::INFO:
			levelStr = CYAN "INFO" RESET;
			break;
		case ::WARNING:
			levelStr = YELLOW "WARNING" RESET;
			break;
		case ::ERROR:
			levelStr = RED "ERROR" RESET;
			break;
		case ::DEBUG:
			levelStr = GREEN "DEBUG" RESET;
			break;
		}

		std::cout << BRIGHT_BLACK
				  << file << ":" << line << " (" << func << ") "
				  << RESET << "[" << levelStr << "] "
				  << message << std::endl;
	}
};

