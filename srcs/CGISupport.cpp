#include <cstdlib> // for getenv
#include "sys/wait.h" // waitpid

#include "Webserv.hpp"
#include "ServerManager.hpp"

std::string unchunkBody(const std::string& raw_body);

std::string run_cgi_script(const HttpRequest &req, const std::string &script_path)
{
    // create a pipe: pipefd[0] = read end, pipefd[1] = write end.
    // The CGI script will write its output to this page.
    int pipefd[2];

    if (pipe(pipefd) == -1)
        return ("<h1>500 Internal Server Error</h1>");

    // Forks a child process to run the CGI script.
    // Parent will read from the pipe, child will execve() the script.
    pid_t pid = fork();
    if (pid < 0)
        return ("<h1>500 Internal Server Error</h1>");

    if (pid == 0)
    {
        // Child process

        // Redirect STDOUT to write-end of pipe, close unused read-end
        dup2(pipefd[1], STDOUT_FILENO); // CGI writes to pipe
        close(pipefd[0]);               // Close read end in child

        // setup environment variables
        char *env[] = {
            strdup(("REQUEST_METHOD=" + req.method).c_str()),
            strdup(("QUERY_STRING=" + req.query_string).c_str()),
            strdup(("SCRIPT_NAME=" + req.path).c_str()),
            strdup("SERVER_PROTOCOL=HTTP/1.1"),
            NULL
        };

        // setup arguments
        char *args[] = {
            strdup((char *)"/usr/bin/python3"), // interpreter
            strdup((char *)script_path.c_str()), // script path
            NULL
        };

        execve(args[0], args, env); // replaces current process
        exit(1); // if exec fails
    }

    // parent
    close(pipefd[1]); // close write-end
    waitpid(pid, NULL, 0); // Wait for child to finish

    char buffer[1024];
    std::string output;
    int n;
    // Read from pipe
    while ((n = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
        output.append(buffer, n);
    }
    close(pipefd[0]);
    return (output);
}

/*  unchunckBody() - removing the chunked Format
    The server reads a raw body like this(when sent from curl or browsed using
    Transfer-Encoding: chunked ):
    -Reads the hex chunk size
    -Reads that many bytes
    -Appends them to a std::string
    -Repeats until a 0 chunk is found (end of body) */
std::string unchunkBody(const std::string& raw_body) {
    std::istringstream iss(raw_body);
    std::string unchunked, line;
    while (std::getline(iss, line)) {
        std::stringstream size_stream(line);
        int chunk_size = 0;
        size_stream >> std::hex >> chunk_size;

        if (chunk_size == 0)
            break;

        char* buffer = new char[chunk_size];
        iss.read(buffer, chunk_size);
        unchunked.append(buffer, chunk_size);
        delete[] buffer;

        // Consume trailing \r\n after chunk
        iss.ignore(2);
    }
    return (unchunked);
}