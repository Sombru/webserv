#include "ParserManager.hpp"

int ParserManager::parseRequest(const Client& client)	
{
	std::istringstream stream(client.getRaw_request());
	std::string line;

	// Parse the request line: "GET /path?query=string HTTP/1.1"
	if (std::getline(stream, line))
	{
		std::istringstream line_stream(line);
		line_stream >> this->request.method;

		std::string full_path;
		line_stream >> full_path;

		size_t q = full_path.find('?');
		if (q != std::string::npos)
		{
			this->request.path = full_path.substr(0, q);
			this->request.query_string = full_path.substr(q + 1);
			this->request.query_params = parse_query(this->request.query_string);
		}
		else
		{
			this->request.path = full_path;
		}

		line_stream >> this->request.version;
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
			this->request.headers[key] = value;
		}
	}
	std::cout << this->request;
	// req.path = INDEX; // this will be something else idk what for now
	return 0;
}
