// Logger.hpp
#pragma once

#define LOG_INFO "\033[32m"      // Green
#define LOG_WARNING "\033[33m"   // Yellow
#define LOG_ERROR "\033[31m"     // Red
#define LOG_DEBUG "\033[36m"     // Cyan
#define LOG_TIMESTAMP "\033[90m" // Bright Black (Gray)
#define LOG_RESET "\033[0m"      // Reset to default

#include "HTTP.hpp"
#include "Webserv.hpp"
#include "Config.hpp"
#include <ctime>

enum LogLevel
{
    INFO,
    WARNING,
    ERROR,
    DEBUG
};

class Logger
{
public:
    // static void log(LogLevel level, const std::string& message);

    // Clean info function for messages
    template <typename T>
    static void info(const T &message);
    template <typename T>
    static void warning(const T &message);
    template <typename T>
    static void error(const T &message);
    template <typename T>
    static void debug(const T &message);

    // Overloaded version that accepts extra parameters
    static void info(const std::string &message, const std::string &serverName, const std::string &host, int port);

private:
    static std::string getTimestamp();
    // static std::string levelToString(LogLevel level);
};

// Implementation moved to Logger.cpp
std::ostream &operator<<(std::ostream &os, const Token &token);
std::ostream &operator<<(std::ostream &os, const std::vector<Token> &tokens);
std::ostream &operator<<(std::ostream &os, const ServerConfig &coserver);
std::ostream &operator<<(std::ostream &os, const LocationConfig &location);
std::ostream &operator<<(std::ostream &os, const HttpRequest &req);
// #include "../utils/Logger.cpp"


template <typename T>
void Logger::info(const T &message)
{
    std::cout << LOG_TIMESTAMP << "[" << getTimestamp() << "] " << LOG_RESET
              << LOG_INFO << "[INFO] " << LOG_RESET
              << message << std::endl;
}

template <typename T>
void Logger::warning(const T &message)
{
    std::cout << LOG_TIMESTAMP << "[" << getTimestamp() << "] " << LOG_RESET
              << LOG_WARNING << "[WARNING] " << LOG_RESET
              << message << std::endl;
}

template <typename T>
void Logger::error(const T &message)
{
    std::cout << LOG_TIMESTAMP << "[" << getTimestamp() << "] " << LOG_RESET
              << LOG_ERROR << "[ERROR] " << LOG_RESET
              << message << std::endl;
}

template <typename T>
void Logger::debug(const T &message)
{
    std::cout << LOG_TIMESTAMP << "[" << getTimestamp() << "] " << LOG_RESET
              << LOG_DEBUG << "[DEBUG] " << LOG_RESET
              << message << std::endl;
}
