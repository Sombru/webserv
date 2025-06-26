#ifndef LIBRARIES_HPP
# define LIBRARIES_HPP

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdexcept>
#include <map>
#include <stdlib.h>
#include <vector>
#include <cctype>


// preprocessor for hash map
#define INDEX "webpages/index.html"


// TODO add a httpresponse structure

void openFile(std::ifstream& file, const std::string& filename);
void validateFormat(std::string filename);
void validateStartTitle(const std::string& line);
void validateTitles(std::ifstream &file);
void validateBraces(std::string filename);
void validateFormat(std::string filename);

#include "ParserManager.hpp"
#endif