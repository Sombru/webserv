// Example HTTP Request Handler Enhancement

#include "Server.hpp"
#include <sstream>

struct HttpRequest 
{
    std::string method;
    std::string uri;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;
    bool isValid;
    
    HttpRequest() : isValid(false) {}
};

class HttpParser 
{
public:
    static HttpRequest parseRequest(const std::string& rawRequest) 
    {
        HttpRequest request;
        std::istringstream stream(rawRequest);
        std::string line;
        
        // Parse request line: METHOD URI VERSION
        if (std::getline(stream, line)) 
        {
            std::istringstream lineStream(line);
            lineStream >> request.method >> request.uri >> request.version;
            
            if (!request.method.empty() && !request.uri.empty()) 
            {
                request.isValid = true;
            }
        }
        
        // Parse headers
        while (std::getline(stream, line) && line != "\r" && !line.empty()) 
        {
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) 
            {
                std::string key = line.substr(0, colonPos);
                std::string value = line.substr(colonPos + 1);
                // Trim whitespace
                while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
                    value.erase(0, 1);
                request.headers[key] = value;
            }
        }
        
        // Read body (if any)
        std::string bodyLine;
        while (std::getline(stream, bodyLine)) 
        {
            request.body += bodyLine + "\n";
        }
        
        return request;
    }
    
    static std::string createResponse(int statusCode, const std::string& statusText, 
                                    const std::string& body, 
                                    const std::map<std::string, std::string>& headers = std::map<std::string, std::string>()) 
    {
        std::ostringstream response;
        response << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
        
        // Add headers
        for (std::map<std::string, std::string>::const_iterator it = headers.begin(); 
             it != headers.end(); ++it) 
        {
            response << it->first << ": " << it->second << "\r\n";
        }
        
        response << "Content-Length: " << body.length() << "\r\n";
        response << "Connection: close\r\n";
        response << "\r\n";
        response << body;
        
        return response.str();
    }
};

// Enhanced handleConnection method
void Server::handleConnection(int fd)
{
    char buffer[4096];
    std::string requestData;

    while (true)
    {
        ssize_t bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                break; // No more data
            }
            ERROR("Error reading from client " + intToString(fd) + ": " + errstr);
            closeClient(fd);
            return;
        }

        if (bytesRead == 0)
        {
            INFO("Client " + intToString(fd) + " disconnected");
            closeClient(fd);
            return;
        }

        buffer[bytesRead] = '\0';
        requestData += buffer;
        
        // Check if we have a complete HTTP request
        if (requestData.find("\r\n\r\n") != std::string::npos)
        {
            HttpRequest request = HttpParser::parseRequest(requestData);
            
            if (request.isValid)
            {
                handleHttpRequest(request, fd);
            }
            else
            {
                // Send 400 Bad Request
                std::string response = HttpParser::createResponse(400, "Bad Request", 
                    "<html><body><h1>400 Bad Request</h1></body></html>");
                send(fd, response.c_str(), response.length(), 0);
            }
            
            closeClient(fd);
            return;
        }
    }
}

void Server::handleHttpRequest(const HttpRequest& request, int fd)
{
    std::string responseBody;
    int statusCode = 200;
    std::string statusText = "OK";
    
    if (request.method == "GET")
    {
        // Handle GET request based on URI and server config
        if (request.uri == "/")
        {
            responseBody = "<html><body><h1>Welcome to " + serverConfig.name + "</h1></body></html>";
        }
        else
        {
            // Try to serve file from server root
            std::string filePath = serverConfig.root + request.uri;
            responseBody = readFile(filePath);
            
            if (responseBody == BADFILE)
            {
                statusCode = 404;
                statusText = "Not Found";
                responseBody = "<html><body><h1>404 Not Found</h1></body></html>";
            }
        }
    }
    else if (request.method == "POST")
    {
        // Handle POST request
        responseBody = "<html><body><h1>POST received</h1><p>Body: " + request.body + "</p></body></html>";
    }
    else
    {
        statusCode = 405;
        statusText = "Method Not Allowed";
        responseBody = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
    }
    
    std::map<std::string, std::string> headers;
    headers["Content-Type"] = "text/html";
    headers["Server"] = serverConfig.name;
    
    std::string response = HttpParser::createResponse(statusCode, statusText, responseBody, headers);
    send(fd, response.c_str(), response.length(), 0);
}

void Server::closeClient(int fd)
{
    close(fd);
    // Remove from epoll and client map would be handled by ServerManager
}
