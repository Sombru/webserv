#include "HTTP.hpp"
#include "Webserv.hpp"
#include "Config.hpp"
#include "Logger.hpp"

HttpResponse run_cgi_script(const HttpRequest &request, const ServerConfig& server, const std::string& fs_path);

// helper function Mime so the browser would know what kind of file is being served:
static std::string getMimeType(const std::string &path) {
	if (path.size() >= 4 && path.substr(path.size() - 4) == ".pdf") 
		return "application/pdf"; 
	if (path.size() >= 4 && path.substr(path.size() - 4) == ".png")
		return "image/png";
	if (path.size() >= 4 && path.substr(path.size() - 4) == ".jpg")
		return "image/jpeg";
	if (path.size() >= 5 && path.substr(path.size() - 5) == ".html")
		return "text/html";
	return "application/octet-stream"; // default for unknown files
}

HttpResponse GET(const HttpRequest &request, const ServerConfig &server)
{
	std::string fs_path = server.root;
	fs_path += request.best_location->root;

	std::vector<std::string> contents = getLocationContents(fs_path);
	std::string body;

	if (request.best_location->name == "/cgi-bin/")
		return run_cgi_script(request, server, fs_path);

	if (!request.target_file.empty())
	{
		fs_path += request.target_file;

		Logger::error("Print me this");
		// if this is the /upload/ location, serve the file as binary
		if (request.best_location->name == "/uploads/") {
			std::ifstream file(fs_path.c_str(), std::ios::binary);
			if (!file.is_open()) {
				return buildErrorResponse(404, server);
			}

			std::ostringstream buffer;
			buffer << file.rdbuf();
			std::string body = buffer.str();

			HttpResponse res;
			res.status_code = OK;
			res.headers["Content-Length"] = intToString(body.size());
			res.headers["Content-Type"] = getMimeType(fs_path);
			res.body = body;
			return res;
		}
		// otherwise execure regular HTML/text read
		body = readFile(fs_path);
		if (body == "BAD")
			return buildErrorResponse(NOTFOUD, server);

		// Inject file list if the target is the main page
		if (request.target_file == "index.html") {
			std::string fileListHtml = generateFileListHtml("uploads");
			size_t pos = body.find("{{file_list}}");
			if (pos != std::string::npos)
				body.replace(pos, 14, fileListHtml);  // 14 = length of "{{file_list}}"
		}

		return buildSuccessResponse(OK, body);
	}
	else if (!contents.empty())
	{
		if (request.best_location->index.empty())
			return buildErrorResponse(FORBIDEN, server);

		body = readFile(fs_path + request.best_location->index);

		// Same logic for index.html when not specified in URL
		if (request.best_location->index == "index.html") {
			std::string fileListHtml = generateFileListHtml("uploads");
			size_t pos = body.find("{{file_list}}");
			if (pos != std::string::npos)
				body.replace(pos, 14, fileListHtml);
		}

		if (request.best_location->autoindex)
		{
			body += "<ul>";
			for (size_t i = 0; i < contents.size(); ++i)
				body += "<li>" + contents[i] + "</li>";
			body += "</ul>";
			return buildSuccessResponse(OK, body);
		}
		return buildSuccessResponse(OK, body);
	}
	return buildErrorResponse(NOTFOUD, server);
}
