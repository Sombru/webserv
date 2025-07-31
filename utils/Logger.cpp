// Logger.cpp
#include "Logger.hpp"
#include "Config.hpp"
#include "HTTP.hpp"

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


std::ostream &operator<<(std::ostream &os, const HttpRequest &req)
{
	os << "=== HTTP REQUEST ===\n";
	os << "Method:  " << req.method << "\n";
	os << "Path:    " << req.path << "\n";
	os << "Version: " << req.version << "\n";
	os << "Location: " << (req.best_location ? req.best_location->name :"NOTFOUND") << '\n';

	os << "Query Parameters:\n";
	for (std::map<std::string, std::string>::const_iterator it = req.query_params.begin(); it != req.query_params.end(); ++it)
	{
		os << "  " << it->first << " = " << it->second << "\n";
	}

	os << "Headers:\n";
	for (std::map<std::string, std::string>::const_iterator it = req.headers.begin(); it != req.headers.end(); ++it)
	{
		os << "  " << it->first << ": " << it->second << "\n";
	}

	os << "Body;\n";
	os << req.body;
	os << '\n';
	os << "====================\n";
	return os;
}

// std::ostream &operator<<(std::ostream& os, const std::vector<std::string>& vector)
// {
// 	for (std::vector<std::string>::const_iterator it = vector.begin(); it != vector.end(); ++it)
// 	{
// 		os << *it << " ";
// 	}
// 	return os;
// }