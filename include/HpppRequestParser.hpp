// HttpRequestParser.hpp
#pragma once
#include "Webserv.hpp"
#include <sstream>
#include <iostream>

class HttpRequestParser
{
public:
    static HttpRequest parse(const std::string &raw_request)
    {
        HttpRequest req;
        std::istringstream stream(raw_request);
        std::string line;

        // Parse request line
        if (std::getline(stream, line))
        {
            std::istringstream line_stream(line);
            std::string full_path;

            line_stream >> req.method;
            line_stream >> req.path;
            line_stream >> req.version;

            size_t q = full_path.find('?');
            if (q != std::string::npos)
            {
                req.path = full_path.substr(0, q);
                req.query_string = full_path.substr(q + 1);
                req.query_params = parse_query(req.query_string);
            }
            else
            {
                req.path = full_path;
            }
        }
        // Parse headers
        while (std::getline(stream, line) && line != "\r")
        {
            size_t colon = line.find(':');
            if (colon != std::string::npos)
            {
                std::string key = line.substr(0, colon);
            }
        }
    }
};