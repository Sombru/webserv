#include "HTTP.hpp"
#include "Webserv.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include <sstream>

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
    const LocationConfig* location = request.best_location;

    if (!isMethodAllowed(location, request.method))
        return buildErrorResponse(405, server);

    if (request.body.size() > server.client_max_body_size)
        return buildErrorResponse(413, server);

    if (request.headers.find("Transfer-Encoding") != request.headers.end() &&
        request.headers.at("Transfer-Encoding") == "chunked") {
        if (!unchunkBody(request.body))
            return buildErrorResponse(400, server);
    }

    std::string contentType = request.headers["Content-Type"];
    std::string upload_dir = (location && !location->upload_dir.empty())
                             ? location->upload_dir
                             : "./uploads";
    std::string filepath;
    // response.status_code = 200;
    // response.status_text = getStatusText(200);
    // response.body = "Data received (not saved)";

    //check if we're getting a file 
    if (contentType.find("multipart/form-data") != std::string::npos) {
        Logger::debug("Parsing multipart/form-data");

        size_t boundary_pos = contentType.find("boundary=");

        if (boundary_pos == std::string::npos) {
            Logger::error("No boundary found in Content-Type: " + contentType);
            return buildErrorResponse(400, server);
        }

        std::string boundary = contentType.substr(boundary_pos + 9);
        std::string delimiter = "--" + boundary;
        
        Logger::debug("Parsed boundary: " + boundary);
        Logger::debug("Using delimiter: " + delimiter);

        const std::string& body = request.body;
        size_t start = 0;
        bool fileSaved = false;

        while ((start = body.find(delimiter, start)) != std::string::npos) {
            start += delimiter.size();

            if (body.compare(start, 2, "--") == 0) {
                Logger::debug("End of multipart body reached");
                break;
            }

            if (body.compare(start, 2, "\r\n") == 0)
                start += 2;

            size_t header_end = body.find("\r\n\r\n", start);
            if (header_end == std::string::npos) {
                Logger::error("Couldn't find header end");
                break;
            }

            std::string headers = body.substr(start, header_end - start);
            Logger::debug("Part headers: " + headers);

            size_t disp_start = headers.find("Content-Disposition:");
            if (disp_start == std::string::npos) {
                Logger::warning("No Content-Disposition header, skipping");
                continue;
            }

            size_t filename_pos = headers.find("filename=\"", disp_start);
            if (filename_pos == std::string::npos) {
                Logger::warning("No filename in header, skipping");
                continue;
            }

            filename_pos += 10;
            size_t filename_end = headers.find("\"", filename_pos);
            if (filename_end == std::string::npos) {
                Logger::warning("Malformed filename, skipping");
                continue;
            }

            std::string filename = headers.substr(filename_pos, filename_end - filename_pos);
            Logger::debug("Extracted filename: " + filename);

            if (filename.empty()) {
                Logger::warning("Empty filename, skipping");
                continue;
            }

            size_t content_start = header_end + 4;
            size_t content_end = body.find(delimiter, content_start);
            if (content_end == std::string::npos) {
                Logger::error("Couldn't find end of content block");
                break;
            }

            std::string file_data = body.substr(content_start, content_end - content_start);

            //Strip trailing \r\n
            if (file_data.size() >= 2 &&
                file_data[file_data.size() - 2] == '\r' &&
                file_data[file_data.size() - 1] == '\n') {
                file_data = file_data.substr(0, file_data.size() - 2);
            }

            filepath = upload_dir + "/" + filename;

            Logger::debug("SAving file to: " + filepath);
            int fd = open(filepath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    Logger::error("Failed to open file: " + filepath);
                    return buildErrorResponse(500, server);
                }

                ssize_t written = write(fd, file_data.c_str(), file_data.size());
                close(fd);

                if (written < 0) {
                    Logger::error("Failed to write to file");
                    return buildErrorResponse(500, server);
                }

                Logger::info("Successfully wrote " + intToString(written) + " bytes to file");

                response.status_code = 201;
                response.status_text = getStatusText(201);
                response.body = "File uploaded successfully to " + filepath;
                fileSaved = true;
                break; // Only handle one file
            }
            
            if (!fileSaved) {
                Logger::error("No file was saved during multipart handling");
                return buildErrorResponse(400, server);
            }
        } else {
            Logger::error("Unsupported Content-Type: " + contentType);
            return buildErrorResponse(400, server); // for unsupported media type
        }
    //  {
    //     // fallback for URL-encoded or raw body
    //     std::string filename = "upload_" + intToString(std::time(0));
    //     filepath = upload_dir + "/" + filename;

    //     std::ofstream ofs(filepath.c_str());
    //     if (!ofs)
    //         return buildErrorResponse(500, server);

    //     std::string decoded = urlDecode(request.body);
    //     if (decoded.empty())
    //         ofs << "# file was created empty.\n";
    //     else
    //         ofs << decoded;
    //     ofs.close();

    //     response.status_code = 201;
    //     response.status_text = getStatusText(201);
    //     response.body = "File uploaded successfully to " + filepath;
    // }

    // Common response headers
    response.headers["Content-Length"] = intToString(response.body.size());
    response.headers["Content-Type"] = "text/plain";
    response.version = "HTTP/1.1";

    Logger::debug("show response");
    Logger::error(response.status_code);
    Logger::error("Response body is " + response.body);
    std::string full_response = serialize(response);
    std::cout << "=== RESPONSE SENT ===\n" << full_response << "\n=====================\n";

    return response;
}


// TESTING POST:
// curl -X POST http://127.0.0.3:8080/upload/ -d "Hello World!"
// TO TEST CURL - don't forget to open a separate terminal and use curl there
