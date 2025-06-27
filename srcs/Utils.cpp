#include "include/Webserv.hpp"
#include "Config.hpp"

void openFile(std::ifstream &file, const std::string &filename)
{
	file.open(filename.c_str());
	if (!file.is_open())
		throw std::runtime_error("Failed to open file: " + filename);
}

strHmap parse_query(const std::string &query_string)
{
	strHmap params;
	std::istringstream ss(query_string);
	std::string pair;
	while (std::getline(ss, pair, '&'))
	{
		size_t eq = pair.find('=');
		if (eq != std::string::npos)
			params[pair.substr(0, eq)] = pair.substr(eq + 1);
		else
			params[pair] = ""; // no value
	}
	return params;
}

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
	os << "[";
	for (size_t i = 0; i < tokens.size(); ++i)
	{
		os << tokens[i];
	}
	os << "]\n";
	return os;
}

std::ostream &operator<<(std::ostream &os, const ServerConfig &config)
{
	os << "ServerConfig {\n";
	os << "  host: " << config.host << "\n";
	os << "  port: " << config.port << "\n";
	os << "  server_name: " << config.server_name << "\n";
	os << "  error_page_404: " << config.error_page_404 << "\n";
	os << "  client_max_body_size: " << config.client_max_body_size << "\n";
	os << "  root: " << config.root << "\n";
	os << "  index: " << config.index << "\n";
	os << "  locations: [\n";
	for (size_t i = 0; i < config.locations.size(); ++i)
	{
		const LocationConfig &loc = config.locations[i];
		os <<  loc << "\n";
	}
	os << "  ]\n";
	os << "}";
	return os;
}

std::ostream &operator<<(std::ostream &os, const HttpRequest &req)
{
	os << "=== HTTP REQUEST ===\n";
	os << "Method:  " << req.method << "\n";
	os << "Path:    " << req.path << "\n";
	os << "Version: " << req.version << "\n";

	if (!req.query_string.empty())
	{
		os << "Query:   " << req.query_string << "\n";
		os << "Query Parameters:\n";
		for (strHmap::const_iterator it = req.query_params.begin(); it != req.query_params.end(); ++it)
		{
			os << "  " << it->first << " = " << it->second << "\n";
		}
	}

	os << "Headers:\n";
	for (strHmap::const_iterator it = req.headers.begin(); it != req.headers.end(); ++it)
	{
		os << "  " << it->first << ": " << it->second << "\n";
	}

	os << "====================\n";
	return os;
}

std::ostream &operator<<(std::ostream &os, const LocationConfig &config)
{
	os << "LocationConfig {\n";
	os << "  path: " << config.path << "\n";
	os << "  root: " << config.root << "\n";
	os << "  alias: " << config.alias << "\n";
	os << "  index: " << config.index << "\n";
	os << "  return_path: " << config.return_path << "\n";
	os << "  autoindex: " << (config.autoindex ? "true" : "false") << "\n";

	os << "  allow_methods: [";
	for (size_t i = 0; i < config.allow_methods.size(); ++i)
	{
		os << config.allow_methods[i];
		if (i + 1 < config.allow_methods.size())
			os << ", ";
	}
	os << "]\n";

	os << "  cgi_path: [";
	for (size_t i = 0; i < config.cgi_path.size(); ++i)
	{
		os << config.cgi_path[i];
		if (i + 1 < config.cgi_path.size())
			os << ", ";
	}
	os << "]\n";

	os << "  cgi_ext: [";
	for (size_t i = 0; i < config.cgi_ext.size(); ++i)
	{
		os << config.cgi_ext[i];
		if (i + 1 < config.cgi_ext.size())
			os << ", ";
	}
	os << "]\n";

	os << "}";
	return os;
}