#include <cstdlib> // for getenv

#include "Webserv.hpp"
#include "ServerManager.hpp"
#include "sys/wait.h"

std::string run_cgi_script(const HttpRequest &req, const std::string &script_path)
{
    int pipefd[2];

    if (pipe(pipefd) == -1)
        return ("<h1>500 Internal Server Error</h1>");

    pid_t pid = fork();
    if (pid < 0)
        return ("<h1>500 Intermal Server Error</h1>");

    if (pid == 0)
    {
        // child process
        dup2(pipefd[1], STDOUT_FILENO); // CGI writes to pipe
        close(pipefd[0]);               // Close read end in child

        // setup environment variables
        char *env[] = {
            strdup(("REQUEST_METHOD=" + req.method).c_str()),
            strdup(("QUERY_STRING=" + req.query_string).c_str()),
            strdup(("SCRIPT_NAME=" + req.path).c_str()),
            strdup("SERVER_PROTOCOL=HTTP/1.1"),
            NULL};

        // setup arguments
        char *args[] = {
            strdup((char *)"/usr/bin/python3"),
            strdup((char *)script_path.c_str()),
            NULL};

        execve(args[0], args, env);
        exit(1); // if exec fails
    }

    // parent
    close(pipefd[1]); // close write end
    waitpid(pid, NULL, 0);

    char buffer[1024];
    std::string output;
    int n;
    while ((n = read(pipefd[0], buffer, sizeof(buffer))) > 0)
    {
        output.append(buffer, n);
    }
    close(pipefd[0]);
    return (output);
}