#include "include/Webserv.hpp"

void openFile(std::ifstream& file, const std::string& filename)
{
    file.open(filename.c_str());
    if (!file.is_open())
        throw std::runtime_error("Failed to open file: " + filename);
}


