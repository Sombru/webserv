// Logger.cpp
#include "Logger.hpp"
#include "Config.hpp"
// #include "HTTP.hpp"

std::ostream &operator<<(std::ostream &os, const Token &token)
{
	switch (token.type)
	{
	case WORD:
		os << "WORD";
		break;
	case LBRACE:
		os << "LBRACE";
		break;
	case RBRACE:
		os << "RBRACE";
		break;
	case SEMICOLON:
		os << "SEMICOLON";
		break;
	default:
		os << "UNKNOWN";
		break;
	}
	os << "('" << token.value << "')\n";
	return os;
}

std::ostream &operator<<(std::ostream &os, const std::vector<Token> &tokens)
{
	os << "\n[";
	for (size_t i = 0; i < tokens.size(); ++i)
	{
		os << tokens[i];
	}
	os << "]\n";
	return os;
}

std::string Logger::getTimestamp()
{
	time_t now = time(0);
	tm *localtm = localtime(&now);

	char buf[20];
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtm);

	return (std::string(buf));
}

std::ostream &operator<<(std::ostream &os, const ServerConfig &server)
{
	os << "=== Server configuration ===" << '\n';
	os << "Host: " << server.host << '\n';
	os << "Port: " << server.port << '\n';
	os << "Server Name: " << server.serverNname << '\n';
	os << "Root: " << server.root << '\n';
	os << "Client Max Body Size: " << server.clientMaxBodySize << '\n';
	os << "Error Path: " << server.errorPath << '\n';
	os << "Number of Locations: " << server.locations.size() << '\n';

	for (size_t i = 0; i < server.locations.size(); i++)
	{
		os << "--- Location " << (i + 1) << " ---" << '\n';
		os << server.locations[i];
	}
	os << "=========================" << '\n';
	return os;
}

std::ostream &operator<<(std::ostream &os, const LocationConfig &location)
{
	os << "  Name: " << location.name << '\n';
	os << "  Root: " << location.root << '\n';
	os << "  Index: " << location.index << '\n';
	os << "  Return Path: " << location.returnPath << '\n';
	os << "  Upload Dir: " << location.uploadDir << '\n';
	os << "  Autoindex: " << (location.autoindex ? "on" : "off") << '\n';

	os << "  Allowed Methods: ";
	for (size_t i = 0; i < location.allowedMethods.size(); i++)
	{
		os << location.allowedMethods[i];
		if (i < location.allowedMethods.size() - 1)
			os << ", ";
	}
	os << '\n';

	os << "  CGI mappings: ";
	for (std::map<std::string, std::string>::const_iterator it = location.cgi.begin();
		 it != location.cgi.end(); ++it)
	{
		os << it->first << " -> " << it->second << "; ";
	}
	os << '\n';
	return os;
}

// std::ostream &operator<<(std::ostream &os, const ServerConfig &config)
// {
// 	os << "ServerConfig {\n";
// 	os << "  host: " << config.host << "\n";
// 	os << "  port: " << config.port << "\n";
// 	os << "  server_name: " << config.server_name << "\n";
// 	os << "  error_pages_dir: " << config.error_pages_dir << "\n";
// 	os << "  client_max_body_size: " << config.client_max_body_size << "\n";
// 	os << "  root: " << config.root << "\n";
// 	os << "  default location: " << config.locations[config.default_location_index].name << "\n";
// 	os << "  locations: {\n";
// 	for (size_t i = 0; i < config.locations.size(); ++i)
// 	{
// 		const LocationConfig &loc = config.locations[i];
// 		os << loc << "\n";
// 	}
// 	os << "  }\n";
// 	os << "}";
// 	return os;
// }

// std::ostream &operator<<(std::ostream &os, const LocationConfig &config)
// {
// 	os << "LocationConfig {\n";
// 	os << "  name: " << config.name << "\n";
// 	os << "  root: " << config.root << "\n";
// 	os << "  index: " << config.index << "\n";
// 	os << "  return_path: " << config.return_path << "\n";
// 	os << "  autoindex: " << (config.autoindex ? "true" : "false") << "\n";

// 	os << "  allow_methods: [";
// 	for (size_t i = 0; i < config.allow_methods.size(); ++i)
// 	{
// 		os << config.allow_methods[i];
// 		if (i + 1 < config.allow_methods.size())
// 			os << ", ";
// 	}
// 	os << "]\n";

// 	os << "  cgi_path: [";
// 	for (size_t i = 0; i < config.cgi_path.size(); ++i)
// 	{
// 		os << config.cgi_path[i];
// 		if (i + 1 < config.cgi_path.size())
// 			os << ", ";
// 	}
// 	os << "]\n";

// 	os << "  cgi_ext: [";
// 	for (size_t i = 0; i < config.cgi_ext.size(); ++i)
// 	{
// 		os << config.cgi_ext[i];
// 		if (i + 1 < config.cgi_ext.size())
// 			os << ", ";
// 	}
// 	os << "]\n";

// 	os << "}";
// 	return os;
// }

// std::ostream &operator<<(std::ostream &os, const HttpRequest &req)
// {
// 	os << "=== HTTP REQUEST ===\n";
// 	os << "Method:  " << req.method << "\n";
// 	os << "Path:    " << req.req_path << "\n";
// 	os << "File system path:    " << req.fs_path << "\n";
// 	os << "Version: " << req.version << "\n";

// 	if (!req.query_string.empty())
// 	{
// 		os << "Query:   " << req.query_string << "\n";
// 		os << "Query Parameters:\n";
// 		for (std::map<std::string, std::string>::const_iterator it = req.query_params.begin(); it != req.query_params.end(); ++it)
// 		{
// 			os << "  " << it->first << " = " << it->second << "\n";
// 		}
// 	}

// 	os << "Headers:\n";
// 	for (std::map<std::string, std::string>::const_iterator it = req.headers.begin(); it != req.headers.end(); ++it)
// 	{
// 		os << "  " << it->first << ": " << it->second << "\n";
// 	}

// 	os << "Body;\n";
// 	os << req.body;
// 	os << '\n';
// 	os << "====================\n";
// 	return os;
// }

// std::ostream &operator<<(std::ostream& os, const std::vector<std::string>& vector)
// {
// 	for (std::vector<std::string>::const_iterator it = vector.begin(); it != vector.end(); ++it)
// 	{
// 		os << *it << " ";
// 	}
// 	return os;
// }