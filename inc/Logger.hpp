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
    std::cout << "[" << getTimestamp() << "] "
              << "[" << "INFO" << "] "
              << message << std::endl;
}

template<typename T>
void Logger::warning(const T& message) {
    std::cout << "[" << getTimestamp() << "] "
              << "[" << "WARNING" << "] "
              << message << std::endl;
}

template<typename T>
void Logger::error(const T& message) {
    std::cout << "[" << getTimestamp() << "] "
              << "[" << "ERROR" << "] "
              << message << std::endl;
}

template<typename T>
void Logger::debug(const T& message) {
    std::cout << "[" << getTimestamp() << "] "
              << "[" << "DEBUG" << "] "
              << message << std::endl;
}

#endif