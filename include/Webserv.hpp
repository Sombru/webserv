#ifndef LIBRARIES_HPP
# define LIBRARIES_HPP

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdexcept>
#include <map>
#include <stdlib.h>
#include <vector>
#include <cctype>

// preprocessor for hash map
#define strHmap std::map<std::string, std::string>
#define INDEX "webpages/index.html"

struct HttpRequest
{
	std::string method;
	std::string path;
	std::string query_string;
	std::string version;
	strHmap query_params;
	strHmap headers;
};

struct HttpResponse 
{
	std::string version;        // e.g. "HTTP/1.1"
	int status_code;            // e.g. 200
	std::string status_text;    // e.g. "OK"
	strHmap headers;
	std::string body;
};


// TODO add a httpresponse structure

void openFile(std::ifstream& file, const std::string& filename);
strHmap parse_query(const std::string &query_string);
HttpRequest parseRequest(const std::string &raw_request);
std::ostream &operator<<(std::ostream &os, const HttpRequest &req);
void validateFormat(std::string filename);
void validateStartTitle(const std::string& line);
void validateTitles(std::ifstream &file);
void validateBraces(std::string filename);
void validateFormat(std::string filename);

struct ServerConfig
{
    unsigned int                port;
    std::string                 host;
    std::string                 server_name;
    std::map<int, std::string>  error_pages;
    unsigned int                client_max_body_size;
    std::string                 root;
    std::string                 index;

    void saveData(const std::string& filename)
    {
        std::string line;
        std::ifstream file;
        openFile(file, filename);
        while (std::getline(file, line))
        {
            std::stringstream ss(line);
            std::string key, value;
            ss >> key >> value;
            if (key == "listen")
            {
                std::cout << "saving port on" << std::atoi(value.c_str()) << std::endl;
                port = std::atoi(value.c_str());
            }
                
            else if (key == "host")
                host = value;
            else if (key == "server_name")
                server_name = value;
            else if (key == "client_max_body_size")
                client_max_body_size = std::atoi(value.c_str());
            else if (key == "root")
                root = value;
            else if (key == "index")
                index = value;
            else if (key == "error_page")
            {
                int code;
                std::string path;
                ss >> code >> path;
                if (!path.empty())
                    error_pages[code] = path;
            }
        }
    }

    void validateData()
    {
        if (port == 0 || host.empty() || server_name.empty() || client_max_body_size == 0 || root.empty() || index.empty())
            throw std::invalid_argument("Invalid config");
        for (std::map<int, std::string>::const_iterator it = error_pages.begin(); it != error_pages.end(); ++it)
        {
            if (it->second.empty())
                throw std::invalid_argument("Invalid config");
        }
    }
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

};

struct Config
{
    ServerConfig server;
    LocationConfig location;

    void parse(const std::string& filename)
    {
        validateFormat(filename);
        server.saveData(filename);
        server.validateData();
    }
};


#endif