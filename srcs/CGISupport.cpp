#include <cstdlib> // for getenv
#include <sys/wait.h> //waitpid

#include "Logger.hpp"
#include "ServerManager.hpp"
#include "Webserv.hpp"
#include "HTTP.hpp"

/*  When a request matches a CGI-enabled path(e.g. /cgi-bin/script.py),
    the server doesn't serve static HTML, bu launches the script 
    as a subprocess.
    The script takes input from the request and returns an 
    HTTP-formatted output.*/

/*  CGI Workflow Overview
    HTTP Request is received → matched to a CGI location.

    Server sets up CGI environment variables.

    Fork and exec the script (e.g. Python or Bash).

    Write the request body to the script’s stdin (for POST).

    Read the script’s output from its stdout.

    Parse the output (headers + body).

    Send it back to the client.*/

std::string run_cgi_script(const HttpRequest& request, const std::string& script_path,const ServerConfig& server) {
    // create a pipe: pipefd[0] == read end, pipefd[1] = write end.
    // The CGI script will write its output to the page.
    HttpResponse response;
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        response.status_code = 500;
        response.status_text = getStatusText(response.status_code);
        return (loadTemplateErrorPage(response.status_code, response.status_text));
    }
    
    // Forks a child process to run the CGI script.
    // Parent will read from the pipe, child will execve() the script.
    pid_t pid = fork();
    
    if (pid < 0) {
        response.status_code = 500;
        response.status_text = getStatusText(response.status_code);
        return (loadTemplateErrorPage(response.status_code, response.status_text));
    }

    if (pid == 0) {
        // Child process

        // Redirect STDOUT to write-end of pipe, close unused read-end
        dup2(pipefd[1], STDOUT_FILENO); // CGI writes to pipe
        close(pipefd[0]);               // Close read end in child

        // setup enviroment variables
        char *env[] = {
            strdup(("REQUEST_METHOD=" + request.method).c_str()),
            strdup(("QUERY_STRING=" + request.query_string).c_str()),
            strdup(("SCRIPT_NAME=" + request.path).c_str()),
            strdup(("SERVER_PROTOCOL=HTTP/1.1")),
            NULL
        };

        // setup arguments
        char *args[] = {
            strdup((char*)"/usr/bin/python3"), // interpreter
            strdup((char*)script_path.c_str()), // script path
            NULL
        };

        execve(args[0], args, env); // replaces current process
        Logger::debug("execve failed");
        _exit(1); // safe exit for child
        // kill(0, SIGINT)
        /* _exit(1) does a raw exit without affecting parent/global
            state. 42 norms allow _exit(), as it is a syscall-level
            and safe in fork()ed child.
            42 norms doesn't allow using exit(1), because it terminates
            the entire process, which in this multiclient server is bad. */
    }
    // parent
    close(pipefd[1]); // close write-end
    waitpid(pid, NULL, 0); // Wait for child to finish

    char buffer[1024];
    std::string output;
    int bytes_read;
    // read from pipe
    while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
        output.append(buffer, bytes_read);
    }
    close(pipefd[0]);
    return (output);
    }