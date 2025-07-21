#include "HTTP.hpp"
#include "Webserv.hpp"
#include "Config.hpp"
#include "Logger.hpp"

HttpResponse POST(const HttpRequest &request, const ServerConfig &server)
{
	// Validate POST is allowed in this location
    // if !allowedMethod"post" -> buildError 405-Method not allowed

    // Check body size
    if (request.body.size() > server.client_max_body_size) {
        return buildErrorResponse(413, server);
    }

    // If body is chunked, dechunk it
    std::string body = request.body;
    if (request.headers["..."] == "chunked") {
        if (!unchunkBody(body)) {
            return buildErrorResponse(400, server);
        }
    }
}

/* Example of a chunked body    "4\r\n"
                                "Wiki\r\n"
                                "5\r\n"
                                "pedia\r\n"
                                "0\r\n"
                                "\r\n"*/
// read the chunk size data though the end of the body and then rebuild the full unchunked body
bool unchunkBody(std::string &body) {
    std::string unchunked_body;
    size_t pos_in_body = 0;

    while (pos_in_body < body.length()) {
        // find the position of the first "\r\n"
        size_t pos_of_the_end = body.find("\r\n", pos_in_body);
        if (pos_of_the_end == std::string::npos)
            return (false);

        // read chunk size as hex
        std::string size_str = body.substr(pos_in_body, pos_of_the_end - pos_in_body);
        int chunk_size = std::strtol(size_str.c_str(), NULL, 16); //strtol() converting a c-str into a long int. 16->hexadec to decimal 
        // conveting is done to know how much data is coming
        if (chunk_size == 0) { // used as - found the end of the message
            // Expect final "\r\n"
            if (body.substr(pos_of_the_end + 2, 2) != "\r\n")
                return false;
            break; // if "0\r\n" <- is breaking the loop
        }

        // move past the "\r\n" and continue reading
        pos_in_body = pos_of_the_end + 2;

        // checking if we have enough bytes
        if (pos_in_body + chunk_size > body.size())
            return false;

        // now extracting and appending the chunk content to the final string
        unchunked_body.append(body.substr(pos_in_body, chunk_size));
        pos_in_body += chunk_size;

        // checking if each chunk ends with "\r\n" + and moving past it
        if (body.substr(pos_in_body, 2) != "\r\n")
            return false;
        
            pos_in_body += 2;
    }

    body = unchunked_body; // Replacing the original string with the unchunked string
    return true;
}