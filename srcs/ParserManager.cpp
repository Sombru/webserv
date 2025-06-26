#include "ParserManager.hpp"

ParserManager::ParserManager(DataType type, const std::string& data)
{
	switch(type)
	{
		case(CONFIG):
			config.parse(data);
			break;
		case(RESPONSE):
			//response.parse(data);
			//std::cout << "here is the rule for parsing a response" << std::endl;
			break;
		case(REQUEST):
			request.parse(data);
			std::cout << "here is the rule for parsing a request" << std::endl;
			break;
	}
}

void Config::parse(const std::string& filename)
{
	validateFormat(filename);
	server.saveData(filename);
	server.validateData();
}

void ServerConfig::saveData(const std::string& filename)
{
	std::string line;
	std::ifstream file;
	openFile(file, filename);
	while (std::getline(file, line))
	{
		std::stringstream ss(line);
		std::string key, value;
		ss >> key >> value;
		if (!value.empty() && value[value.length() - 1] == ';')
			value = value.substr(0, value.length() - 1);
		if (key == "listen")
			port = std::atoi(value.c_str());
		else if (key == "host")
			host = value;
		else if (key == "server_name")
			server_name = value;
		else if (key == "client_max_body_size")
			client_max_body_size = std::atoi(value.c_str());
		else if (key == "root")
			root = value;
		else if (key == "index")
			index = value;
		else if (key == "error_page")
		{
			int code;
			std::string path;
			ss >> code >> path;
			if (!path.empty())
				error_pages[code] = path;
		}
	}
}

void ServerConfig::validateData()
{
	if (port == 0 || host.empty() || server_name.empty() || client_max_body_size == 0 || root.empty() || index.empty())
		throw std::invalid_argument("Invalid config");
	for (std::map<int, std::string>::const_iterator it = error_pages.begin(); it != error_pages.end(); ++it)
	{
		if (it->second.empty())
			throw std::invalid_argument("Invalid config");
	}
}

std::map<std::string, std::string> HttpRequest::parse_query(const std::string &query_string)
{
	std::map<std::string, std::string> params;
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


void HttpRequest::parse(const std::string& data)
{
	std::istringstream stream(data);
	std::string line;

	if (std::getline(stream, line))
	{
		std::istringstream line_stream(line);
		line_stream >> method;

		std::string full_path;
		line_stream >> full_path;

		size_t q = full_path.find('?');
		if (q != std::string::npos)
		{
			path = full_path.substr(0, q);
			query_string = full_path.substr(q + 1);
			query_params = parse_query(query_string);
		}
		else
		{
			path = full_path;
		}

		line_stream >> version;
	}

	while (std::getline(stream, line) && line != "\r")
	{
		size_t colon = line.find(':');
		if (colon != std::string::npos)
		{
			std::string key = line.substr(0, colon);
			std::string value = line.substr(colon + 1);
			if (!value.empty() && value[0] == ' ')
				value = value.substr(1);
			if (!value.empty() && value[value.length() - 1] == '\r')
				value = value.substr(0, value.length() - 1);
			headers[key] = value;
		}
	}
	// req.path = INDEX; // this will be something else idk what for now
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
		for (std::map<std::string, std::string>::const_iterator it = req.query_params.begin(); it != req.query_params.end(); ++it)
		{
			os << "  " << it->first << " = " << it->second << "\n";
		}
	}

	os << "Headers:\n";
	for (std::map<std::string, std::string>::const_iterator it = req.headers.begin(); it != req.headers.end(); ++it)
	{
		os << "  " << it->first << ": " << it->second << "\n";	
	}

	os << "====================\n";
	return os;
}

std::ostream& operator<<(std::ostream& os, const ServerConfig& server) 
{
    os << "ServerConfig {\n";
    os << "  port: " << server.port << "\n";
    os << "  host: " << server.host << "\n";
    os << "  server_name: " << server.server_name << "\n";
    os << "  client_max_body_size: " << server.client_max_body_size << "\n";
    os << "  root: " << server.root << "\n";
    os << "  index: " << server.index << "\n";
    os << "  error_pages:\n";
    for (std::map<int, std::string>::const_iterator it = server.error_pages.begin(); it != server.error_pages.end(); ++it) {
        os << "    " << it->first << " => " << it->second << "\n";
    }
    os << "}\n";
    return os;
}

std::ostream& operator<<(std::ostream& os, const LocationConfig& loc) 
{
    os << "LocationConfig {\n";
    os << "  root: " << loc.root << "\n";
    os << "  autoindex: " << loc.autoindex << "\n";
    os << "  index: " << loc.index << "\n";
    os << "  return_path: " << loc.return_path << "\n";
    os << "  alias: " << loc.alias << "\n";

    os << "  allow_methods: ";
    for (size_t i = 0; i < loc.allow_methods.size(); ++i)
        os << loc.allow_methods[i] << (i + 1 < loc.allow_methods.size() ? ", " : "\n");

    os << "  cgi_path: ";
    for (size_t i = 0; i < loc.cgi_path.size(); ++i)
        os << loc.cgi_path[i] << (i + 1 < loc.cgi_path.size() ? ", " : "\n");

    os << "  cgi_ext: ";
    for (size_t i = 0; i < loc.cgi_ext.size(); ++i)
        os << loc.cgi_ext[i] << (i + 1 < loc.cgi_ext.size() ? ", " : "\n");

    os << "}\n";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Config& config) 
{
    os << "Config {\n";
    os << config.server;
    os << config.location;
    os << "}\n";
    return os;
}
