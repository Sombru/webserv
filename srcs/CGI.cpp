#include "HTTP.hpp"


struct ExecveParams
{
	std::vector<const char *> args;
	std::vector<const char *> env;
};

bool HTTP::isCgiScrit()
{
	if (!request.best_location.cgi.empty())
	{
		if (request.best_location.cgi.find(getFileExtension(request.path)) != request.best_location.cgi.end())
		return true;
	}
	return false;
}

std::vector<char *> buildCgiEnv(const HttpRequest &request)
{
	std::vector<std::string> envStrings; // keep strings alive
	std::vector<char *> env;

	envStrings.push_back("REQUEST_METHOD=" + request.method);
	envStrings.push_back("QUERY_STRING=" + request.query_string);
	envStrings.push_back("SCRIPT_NAME=" + request.target_file);
	envStrings.push_back("SERVER_PROTOCOL=" + request.version);
	envStrings.push_back("REQUEST_BODY=" + request.body);
	envStrings.push_back("REQUEST_LOCATION=" + request.best_location.path);

	for (std::map<std::string, std::string>::const_iterator it = request.headers.begin(); it != request.headers.end(); ++it)
	{
		envStrings.push_back(it->first + "=" + it->second);
	}

	for (size_t i = 0; i < envStrings.size(); ++i)
		env.push_back(const_cast<char *>(envStrings[i].c_str()));

	env.push_back(NULL);

	std::vector<char *> envCopy;
	for (size_t i = 0; i < env.size() - 1; ++i)
		envCopy.push_back(strdup(env[i])); // duplicate
	envCopy.push_back(NULL);

	return envCopy; 
}

void HTTP::launchExecve(int inpipe[2], int outpipe[2], const std::string &fspath, const std::string &interpreter)
{
		// Child
	dup2(inpipe[0], STDIN_FILENO);
	dup2(outpipe[1], STDOUT_FILENO);
	close(inpipe[1]);
	close(outpipe[0]);
	// Prepare argv
	std::vector<char *> argv;
	argv.reserve(3);
	argv.push_back((char *)interpreter.c_str());
	argv.push_back((char *)fspath.c_str());
	argv.push_back(NULL);
	// Minimal environment for CGI
	std::vector<char *> env = buildCgiEnv(request);
	execve(interpreter.c_str(), argv.data(), env.data());
	// If exec fails
	_exit(1);
}

// Execute CGI script using interpreter (if provided).
// scriptPath: full filesystem path to script
// interpreter: path to interpreter executable, or empty if script is executable itself
// requestBody: body to pass to CGI via stdin (for POST)
void HTTP::executeCgi(const std::string &fspath, const std::string &interpreter, const std::string &requestBody)
{
	int inpipe[2];
	int outpipe[2];
	// DEBUG(interpreter);
	// DEBUG(fspath);
	if (pipe(inpipe) < 0 || pipe(outpipe) < 0)
		return buildErrorRespose(500);

	pid_t pid = fork();
	if (pid < 0)
		return buildErrorRespose(500);

	if (pid == 0)
		launchExecve(inpipe, outpipe, fspath, interpreter);

	// Parent
	close(inpipe[0]);
	close(outpipe[1]);

	// Write request body to child stdin
	ssize_t toWrite = requestBody.size();
	const char *buf = requestBody.c_str();
	while (toWrite > 0)
	{
		ssize_t n = write(inpipe[1], buf, toWrite);
		if (n <= 0) 
			break;
		buf += n;
		toWrite -= n;
	}
	close(inpipe[1]);

	// Read child's stdout
	std::string childOut;
	char tmp[4096];
	ssize_t n;
	while ((n = read(outpipe[0], tmp, sizeof(tmp))) > 0)
	{
		childOut.append(tmp, n);
	}
	close(outpipe[0]);

	int status = 0;
	waitpid(pid, &status, 0);

	if (childOut.empty())
		return buildErrorRespose(502);

	// Parse CGI output headers (simple parser: headers until blank line)
	size_t hdrEnd = childOut.find("\r\n");
	size_t hdrSkip = 2;
	if (hdrEnd == std::string::npos)
	{
		hdrEnd = childOut.find("\n\n");
		hdrSkip = 2;
	}

	if (hdrEnd != std::string::npos)
	{
		std::istringstream hs(childOut.substr(0, hdrEnd));
		std::string line;
		while (std::getline(hs, line))
		{
			if (line.empty() || line == "\r")
				continue;
			size_t colon = line.find(':');
			if (colon != std::string::npos)
			{
				std::string k = line.substr(0, colon);
				std::string v = line.substr(colon + 1);
				if (!v.empty() && v[0] == ' ') v = v.substr(1);
				if (!v.empty() && v[v.size()-1] == '\r') v = v.substr(0, v.size()-1);
				response.headers[k] = v;
			}
		}
		response.body = childOut.substr(hdrEnd + hdrSkip);
	}
	else
	{
		// No headers, entire output is body
		response.body = childOut;
	}

	// Fill status if CGI set Status header
	if (response.headers.count("Status"))
	{
		std::string st = response.headers["Status"];
		size_t sp = st.find(' ');
		if (sp != std::string::npos)
		{
			response.status_code = std::atoi(st.substr(0, sp).c_str());
			response.status_text = st.substr(sp + 1);
		}
		else
			response.status_code = std::atoi(st.c_str());
	}
	else
	{
		if (response.status_code == 0)
			response.status_code = 200;
		if (response.status_text.empty())
			response.status_text = getStatusText(response.status_code);
	}

	// Ensure content-length
	if (response.headers.find("Content-Length") == response.headers.end())
		response.headers["Content-Length"] = intToString(response.body.size());

}
