#pragma once

#include "Webserv.hpp"
#include "HTTP.hpp"

struct EnvBlock {
    std::vector<std::string> storage;
    std::vector<char*> env;

    EnvBlock(const HttpRequest& request) {
        storage.push_back("REQUEST_METHOD=" + request.method);
        storage.push_back("QUERY_STRING=" + request.query_string);
        storage.push_back("SCRIPT_NAME=" + request.path);
        storage.push_back("SERVER_PROTOCOL=HTTP/1.1");

        for (size_t i = 0; i < storage.size(); ++i)
            env.push_back(const_cast<char*>(storage[i].c_str()));
        env.push_back(NULL); // because execve expects null-terminator
    }

    char** get() {
        return env.data();
    }
};