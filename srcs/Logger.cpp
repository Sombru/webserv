#include "Logger.hpp"
#include "Config.hpp"
#include "Webserv.hpp"

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

std::ostream &operator<<(std::ostream &os, const Config &conf)
{
	for (size_t i = 0; i < conf.getConf().size(); ++i)
	{
		os << conf.getConf()[i];
	}
	return os;
}

std::ostream &operator<<(std::ostream &os, const ServerConfig &server)
{
	os << "=== Server configuration ===" << '\n';
	os << "Server Name: " << server.name << '\n';
	os << "Host: " << server.host << '\n';
	os << "Index: " << server.index << '\n';
	os << "Root: " << server.root << '\n';
	os << "Error Page: " << server.errorPage << '\n';
	os << "Client Max Body Size: " << server.clientMaxBodySize << '\n';
	os << "Timeout: " << server.timeout << '\n';
	os << "Max Events: " << server.maxEvents << '\n';
	os << "Number of Locations: " << server.locations.size() << '\n';
	os << "  mime types: ";
	for (std::map<std::string, std::string>::const_iterator it = server.mimeTypes.begin();
		 it != server.mimeTypes.end(); ++it)
	{
		os << it->first << " -> " << it->second << "; ";
	}
	os << '\n';

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
	os << "  Path: " << location.path << '\n';
	os << "  Root: " << location.root << '\n';
	os << "  Alias: " << location.alias << '\n';
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
