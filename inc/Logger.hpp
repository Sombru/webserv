// Logger.hpp
#pragma once 
#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "HTTP.hpp"
#include "Webserv.hpp"
#include <ctime>

enum LogLevel {
    INFO,
    WARNING,
    ERROR,
    DEBUG
};

#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define CYAN    "\033[36m"
#define MAGENTA "\033[35m"
#define RESET   "\033[0m"

class Logger {
    public:
        //static void log(LogLevel level, const std::string& message);
        
        // Clean info function for messages
		template<typename T>
        static void info(const T& message);
		template<typename T>
        static void warning(const T& message);
		template<typename T>
        static void error(const T& message);
		template<typename T>
        static void debug(const T& message);

		// Overloaded version that accepts extra parameters
        static void info(const std::string& message, const std::string& serverName, const std::string& host, int port);
        
    private:
        static std::string getTimestamp();
        //static std::string levelToString(LogLevel level);
};

// Implementation moved to Logger.cpp

std::ostream &operator<<(std::ostream &os, const HttpRequest &req);
std::ostream &operator<<(std::ostream &os, const std::vector<std::string>& );

template<typename T>
void Logger::info(const T& message) {
    std::cout << GREEN << "[" << getTimestamp() << "] "
              << "[" << "INFO" << "] "
              << message << RESET << std::endl;
}

template<typename T>
void Logger::warning(const T& message) {
    std::cout << MAGENTA << "[" << getTimestamp() << "] "
              << "[" << "WARNING" << "] "
              << message << RESET << std::endl;
}

template<typename T>
void Logger::error(const T& message) {
    std::cout << RED << "[" << getTimestamp() << "] "
              << "[" << "ERROR" << "] "
              << message << RESET << std::endl;
}

template<typename T>
void Logger::debug(const T& message) {
    std::cout << CYAN << "[" << getTimestamp() << "] "
              << "[" << "DEBUG" << "] "
              << message << RESET << std::endl;
}

#endif