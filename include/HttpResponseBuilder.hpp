#pragma once
#include "Webserv.hpp"
#include <fstream>
#include <sstream>

class HttpResponseBuilder {
public:
    static HttpResponse build(const HttpRequest& req);
};