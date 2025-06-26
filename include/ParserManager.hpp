#pragma once
#include "include/Webserv.hpp"

enum DataType
{
    CONFIG,
    RESPONSE,
    REQUEST,
};

struct HttpRequest
{
	std::string                         method;
	std::string                         path;
	std::string                         query_string;
	std::string                         version;
	std::map<std::string, std::string>  query_params;
	std::map<std::string, std::string>  headers;

    std::map<std::string, std::string> parse_query(const std::string &query_string);
    void parse(const std::string& data);
};

struct HttpResponse 
{
	std::string                         version;
	int                                 status_code;            
	std::string                         status_text;  
	std::map<std::string, std::string>  headers;
	std::string                         body;

    //void parse_query(const std::string &query_string);
    //void parse(const std::string& data);
};

struct ServerConfig
{
    unsigned int                port;
    std::string                 host;
    std::string                 server_name;
    std::map<int, std::string>  error_pages;
    unsigned int                client_max_body_size;
    std::string                 root;
    std::string                 index;
    
    void saveData(const std::string& data);
    void validateData();
};

struct LocationConfig
{
    std::string                 root;
    std::string                 autoindex;
    std::vector<std::string>    allow_methods;
    std::string                 index;
    std::string                 return_path;
    std::string                 alias;
    std::vector<std::string>    cgi_path;    
    std::vector<std::string>    cgi_ext;

    //void saveData(const std::string& data){}
    //void validateData(){}
};

struct Config
{
    ServerConfig server;
    LocationConfig location;

    void parse(const std::string& data);
};


class ParserManager 
{
public:
    Config config;
    HttpRequest request;
    HttpResponse response;

    ParserManager(DataType type, const std::string& data);
};

std::ostream &operator<<(std::ostream &os, const HttpRequest &req);
std::ostream& operator<<(std::ostream& os, const LocationConfig& loc);
std::ostream& operator<<(std::ostream& os, const ServerConfig& server);
std::ostream& operator<<(std::ostream& os, const Config& config);