#include <cstdlib>	  // for getenv
#include <sys/wait.h> //waitpid

#include "Logger.hpp"
#include "ServerManager.hpp"
#include "Webserv.hpp"
#include "HTTP.hpp"
#include "CGI.hpp"

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

HttpResponse run_cgi_script(const HttpRequest &request, const ServerConfig& server, const std::string& fs_path)
{
	// create a pipe: pipefd[0] == read end, pipefd[1] = write end.
	// The CGI script will write its output to the page.
	Logger::debug("Runnig cgi script");
			Logger::debug(fs_path);
	int pipefd[2];
	if (pipe(pipefd) == -1)
		return buildErrorResponse(500, server);

	// Forks a child process to run the CGI script.
	// Parent will read from the pipe, child will execve() the script.
	pid_t pid = fork();
	if (pid < 0)
		return buildErrorResponse(500, server);

	if (pid == 0)
	{
		// Child process

		// Redirect STDOUT to write-end of pipe, close unused read-end
		dup2(pipefd[1], STDOUT_FILENO); // CGI writes to pipe
		close(pipefd[0]);				// Close read end in child

		//updating the code to a clean, reusable helper function
        // because of the bonus part - handling multiple CGI requests
		// char *env[] = {
		// 	strdup(("REQUEST_METHOD=" + request.method).c_str()),
		// 	strdup(("QUERY_STRING=" + request.query_string).c_str()),
		// 	strdup(("SCRIPT_NAME=" + request.path).c_str()),
		// 	strdup(("SERVER_PROTOCOL=HTTP/1.1")),
		// 	NULL};
        EnvBlock env_block(request);

		// setup arguments

		char *args[] = {
			strdup((char *)"/usr/bin/python3"),	 // interpreter
			strdup((char *)(fs_path + request.target_file).c_str()), // script path
			NULL};

		execve(args[0], args, env_block.get()); // replaces current process
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

	std::string output;
	char buffer[1024];
	int status; // traching child process status

	// using poll same as what we are using in the main loop
	struct pollfd pfd;
	pfd.fd = pipefd[0];	 // read end of pipe
	pfd.events = POLLIN; // we're waiting for input (CGI output)

	int timeout_ms = 2000; // 2 seconds timeout

	int poll_result = poll(&pfd, 1, timeout_ms);

	if (poll_result <= 0)
	{
		// if poll failed
		kill(pid, SIGKILL);
		waitpid(pid, NULL, 0);
		return buildErrorResponse(500, server);
	}

	// Read data from the pipe
	int bytes_read;
	while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer))) > 0)
	{
		output.append(buffer, bytes_read);
	}

	close(pipefd[0]);

	waitpid(pid, &status, 0); // Wait for child to finish

	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
	{
		// in case if script crashes or execve failed
		return buildErrorResponse(500, server);
	}

	return (buildSuccessResponse(200, buffer));
}