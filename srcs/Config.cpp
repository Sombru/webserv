#include "include/Webserv.hpp"

void validateStartTitle(const std::string& line)
{
    std::string server, braces, leftover;
    std::stringstream ss(line);
    ss >> server >> braces;
    if (server != "server" && braces != "{" && (ss >> leftover))
        throw std::invalid_argument("Unexpected data in input title");
}

void validateTitles(std::ifstream &file)
{
    std::string line;
    if (!std::getline(file, line))
        throw std::runtime_error("File is empty or couldn't read line");
    validateStartTitle(line);
}

void validateBraces(std::string filename)
{   
    std::ifstream file;
	openFile(file, filename);
    std::string line;
    std::vector<std::string> braces;
    while (std::getline(file, line))
	{
        if (line.find("{") != std::string::npos)
            braces.push_back("{");   
        else if (line.find("}") != std::string::npos)
            braces.push_back("}");
	}
    if (braces.size() % 2 != 0)
        throw std::invalid_argument("Invalid braces in input title");
    int end = braces.size();
    for (int start = 0; start < end / 2; start++)
    {
        if(braces[start] != "{" && braces[end] != "}")
            throw std::invalid_argument("Invalid braces in input title");
        end--;
    }
}

void validateFormat(std::string filename)
{
	std::ifstream file;
	openFile(file, filename);
    validateTitles(file);
    validateBraces(filename);
}


std::map<std::string, std::string> storeConfig(std::string filename)
{
	std::string line;
	std::ifstream index;
	openFile(index, filename);
	std::map<std::string, std::string> config;
	while (std::getline(index, line))
	{
		std::stringstream ss(line);
		std::string date, value;

		if (std::getline(ss, date, ' ') && std::getline(ss, value))
		{
			if (!date.empty() && !value.empty())
				config.insert(std::make_pair(date, value));
		}
	}
	return config;
}