#include "HTTP.hpp"
#include "Webserv.hpp"
#include "Config.hpp"
#include "Logger.hpp"

static bool isMethodAllowed(const LocationConfig* location, const std::string& method) {
    if (!location)
        return false;
    if (method == "GET") // because it's always allowed
        return true;
    for (size_t i = 0; i < location->allow_methods.size(); ++i) {
        if (location->allow_methods[i] == method)
            return true;
    }
    return false;
}

/* Example of a chunked body    "4\r\n"
                                "Wiki\r\n"
                                "5\r\n"
                                "pedia\r\n"
                                "0\r\n"
                                "\r\n"
                                4\r\n

    this is the format and this means -> Wikipedia

                4\r\nWiki\r\n → 4 bytes: Wiki
                5\r\npedia\r\n → 5 bytes: pedia
                0\r\n\r\n → means END of chunks
*/
// read the chunk size data though the end of the body and then rebuild the full unchunked body
static bool unchunkBody(std::string &body) {
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

HttpResponse POST(HttpRequest request, const ServerConfig &server)
{
    HttpResponse response;

    // Validate POST is allowed in this location
    // if !allowedMethod"post" -> buildError 405-Method not allowed
    const LocationConfig* location = request.best_location;
    if (!isMethodAllowed(location, request.method))
    {
        return buildErrorResponse(405, server);
        // response.status_code = 405;
        // response.status_text = getStatusText(405);
        // return response;
    }

    // Check body size
    if (request.body.size() > server.client_max_body_size) {
        return buildErrorResponse(413, server);
    }

    // If body is transfer as chunked - using unchuked to return it correctly
    std::map<std::string, std::string>::const_iterator it = request.headers.find("Transfer-Encoding");
    if (it != request.headers.end() && it->second == "chunked") {
        if (!unchunkBody(request.body)) {
            return buildErrorResponse(400, server);
        // response.status_code = 400;
        // response.status_text = getStatusText(400);
        // return response;
        }
    }

    // next - Deretmine the upload location -> folder uploads (or fallback)
    std::string upload_dir = location && !location->upload_dir.empty()
                            ? "./uploads":
                            location->upload_dir;
    
    std::string filename = "upload_" + intToString(std::time(0));
    std::string filepath = upload_dir + "/" + filename;

    // next - save body to file
    std::ofstream ofs(filepath.c_str());
    if (!ofs) {
        return buildErrorResponse(500, server);
        // response.status_code = 500;
        // response.status_text = getStatusText(500);
        // return response;
    }
    ofs << request.body;
    ofs.close();

    // finally - return the sucessful response
    response.status_code = 201;
    response.status_text = getStatusText(201);
    response.body = "File uploaded successfully to " + filepath;
    response.headers["Content-Length"] = intToString(response.body.size());
    response.headers["Content-Type"] = "text/plain";

    return response;
}
