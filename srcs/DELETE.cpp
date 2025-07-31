#include "HTTP.hpp"
#include "Webserv.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include <sys/stat.h>

// The client is asking the server to delelte a specific resource/file.
static std::string resolveFilePath(const HttpRequest &request, const ServerConfig &server) {
    const LocationConfig* location = request.best_location;

    // decide root folder: use upload_dir, or root from location, 
    // or root from server
    std::string root;
    if (location && !location->upload_dir.empty()) {
        root = location->upload_dir;
    } else if (location && !location->root.empty()) {
        root = location->root;
    } else {
        root = server.root;
    }

    // get relative path by stripping location.name from request.path
    std::string relativePath = request.path;
    if (location && !location->name.empty() &&
        relativePath.find(location->name) == 0)
    {
        relativePath = relativePath.substr(location->name.length());
    }

    // Trim any loading slashes from relative path
    while(!relativePath.empty() && relativePath[0] == '/')
        relativePath = relativePath.substr(1);

    return root + "/" + relativePath;
}

HttpResponse DELETE(HttpRequest request, const ServerConfig &server) {
    HttpResponse response;

    // get file path from request
    std::string full_path = resolveFilePath(request, server);

    // check if file exists
    struct stat fileStat;
    if (stat(full_path.c_str(), &fileStat) != 0) {
        return buildErrorResponse(404, server); // File not found
    }

    // handling check if in case someone would try to delete directory instead of a file
    if (!S_ISREG(fileStat.st_mode)) {
        return buildErrorResponse(403, server); // Forbidden (trying to delete directory not a file)
    }

    // Try to delelte
    if (remove(full_path.c_str()) != 0) {
        return buildErrorResponse(500, server); // Failed to delete
    }

    response.status_code = 200;
    response.status_text = getStatusText(200);
    response.body = "File deleted: " + request.path;
    response.headers["Content-Length"] = intToString(response.body.size());
    response.headers["Content-Type"] = "text/plain";
    response.version = "HTTP/1.1";

    return response;
}


// how to test it:example
// curl -X DELETE http://127.0.3:8080/upload/upload_1753695010