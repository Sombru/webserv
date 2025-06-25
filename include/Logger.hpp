// Logger.hpp
#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <iomanip>
#include <ctime>
#include <string>
#include "Webserv.hpp"

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
        static void info(const std::string& message);
        
        // Overloaded version that accepts extra parameters
        static void info(const std::string& message, const std::string& serverName, const std::string& host, int port);
        
        static void warning(const std::string& message);
        static void error(const std::string& message);
        static void debug(const std::string& message);
    private:
        static std::string getTimestamp();
        //static std::string levelToString(LogLevel level);
};

#endif