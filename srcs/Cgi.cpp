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

struct ExecveParams
{
	std::vector<const char *> args;
	std::vector<const char *> env;
	std::vector<std::string> env_strings;
	std::vector<std::string> arg_strings;
};

ExecveParams buildExecveParams(const HttpRequest &request, const std::string &fs_path)
{
	ExecveParams params;

	params.env_strings.reserve(6 + request.headers.size() + 2); // buffer
	params.env_strings.push_back("REQUEST_METHOD=" + request.method);
	params.env_strings.push_back("QUERY_STRING=" + request.query_string);
	params.env_strings.push_back("SCRIPT_NAME=" + request.target_file);
	params.env_strings.push_back("SERVER_PROTOCOL=" + request.version);
	params.env_strings.push_back("REQUEST_BODY=" + request.body);
	params.env_strings.push_back("REQUEST_LOCATION=" + request.best_location->name);

	for (std::map<std::string, std::string>::const_iterator it = request.headers.begin(); it != request.headers.end(); it++)
	{
		params.env_strings.push_back(it->first + "=" + it->second);
	}

	params.env.reserve(params.env_strings.size());
	for (size_t i = 0; i < params.env_strings.size(); i++)
	{
		params.env.push_back(params.env_strings[i].c_str());
	}
	params.env.push_back(NULL);

	std::string extension = getFileExtension(request.target_file);
	if (!extension.empty())
	{
		ssize_t index = -1;
		for (size_t i = 0; i < request.best_location->cgi_ext.size(); i++)
		{
			if (request.best_location->cgi_ext[i] == extension)
			{
				index = i;
				break;
			}
		}

		if (index != -1)
		{
			params.arg_strings.reserve(2);
			params.arg_strings.push_back(request.best_location->cgi_path[index]);
			params.arg_strings.push_back(fs_path + request.target_file);

			params.args.reserve(params.arg_strings.size());
			params.args.push_back(params.arg_strings[0].c_str());
			params.args.push_back(params.arg_strings[1].c_str());
			params.args.push_back(NULL);
		}
	}

	return params;
}

void launchExecve(const ExecveParams& params, const int* pipefd)
{
	dup2(pipefd[1], STDOUT_FILENO); // CGI writes to pipe
	close(pipefd[0]);				// Close read end in child
	execve(params.args[0],
		   (char *const *)params.args.data(),
		   (char *const *)params.env.data());
	_exit(1); // safe exit for child
			  /* _exit(1) does a raw exit without affecting parent/global
				  state. 42 norms allow _exit(), as it is a syscall-level
				  and safe in fork()ed child.
				  42 norms doesn't allow using exit(1), because it terminates
				  the entire process, which in this multiclient server is bad. */
}

HttpResponse run_cgi_script(const HttpRequest &request, const ServerConfig &server, const std::string &fs_path)
{
	// create a pipe: pipefd[0] == read end, pipefd[1] = write end.
	// The CGI script will write its output to the page.
	Logger::debug("Runnig cgi script");
	ExecveParams params = buildExecveParams(request, fs_path);

	int pipefd[2];
	if (pipe(pipefd) == -1)
	{
		Logger::error("pipe failed");
		return buildErrorResponse(500, server);
	}
	// Forks a child process to run the CGI script.
	// Parent will read from the pipe, child will execve() the script.
	pid_t pid = fork();
	if (pid < 0)
	{
		Logger::error("fork failed");
		return buildErrorResponse(500, server);
	}

	if (pid == 0)
		launchExecve(params, pipefd);
	// parent
	close(pipefd[1]); // close write-end

	std::string output;
	char buffer[4086];
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
		Logger::error("poll failed CGI");
		kill(pid, SIGKILL);
		waitpid(pid, NULL, 0);
		return buildErrorResponse(500, server);
	}

	// Read data from the pipe
	int bytes_read;
	while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer))) > 0)
		output.append(buffer, bytes_read);

	close(pipefd[0]);

	waitpid(pid, &status, 0); // Wait for child to finish

	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
	{
		// in case if script crashes or execve failed
		Logger::error("execve failed");
		return buildErrorResponse(500, server);
	}

	return (buildSuccessResponse(200, output));
}