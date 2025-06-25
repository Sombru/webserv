// Logger.cpp
#include "Logger.hpp"

// void Logger::log(LogLevel level, const std::string& message) {
//     std::cout << "[" << getTimestamp() << "] "
//               << "[" << levelToString(level) << "] "
//               << message << std::endl;
// }

std::string Logger::getTimestamp() {
    time_t now = time(0);
    tm* localtm = localtime(&now);

    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtm);

    return (std::string(buf));
}

void Logger::info(const std::string& message) {
    std::cout << "[" << getTimestamp() << "] "
              << "[" << "INFO" << "] "
              << message << std::endl;
}


void Logger::info(const std::string&  message, const std::string& serverName, const std::string& host, int port) {
    std::cout << "[" << getTimestamp() << "] " 
              << "[INFO] "
              << message
              << "ServerName[" << serverName << "] "
              << "Host[" << host << "] "
              << "Port[" << port << "]"
              << std::endl;
}

void Logger::warning(const std::string& message) {
    std::cout << "[" << getTimestamp() << "] "
              << "[" << "WARNING" << "] "
              << message << std::endl;
}

void Logger::error(const std::string& message) {
    std::cout << "[" << getTimestamp() << "] "
              << "[" << "ERROR" << "] "
              << message << std::endl;
}

void Logger::debug(const std::string& message) {
    std::cout << "[" << getTimestamp() << "] "
              << "[" << "DEBUG" << "] "
              << message << std::endl;
}

// std::string Logger::levelToString(LogLevel level) {
//     switch (level)
//     {
//     case INFO: return "INFO";
//     case WARNING: return "WARNING";
//     case ERROR: return "ERROR";
//     case DEBUG: return "DEBUG";
//     default: return "UNKNOWN";
//     }
// }