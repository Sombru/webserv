#pragma once
#include "include/Webserv.hpp"

enum ParseType
{
    CONFIG,
    RESPONSE,
    REQUEST,
};

class ParserManager 
{
public:
    Config config;

    ParserManager(ParseType type, const std::string& filename);
};