#!/usr/bin/env python3

import os

# Required CGI header

# Output a simple HTML page
print("<html>")
print("<head><title>CGI Example PY</title></head>")
print("<body>")
print("<h1>Hello from a CGI PYTHON script!</h1>")

# Print some CGI environment info
print("<p>Request Method: {}</p>".format(os.environ.get("REQUEST_METHOD", "")))
print("<p>Query String: {}</p>".format(os.environ.get("QUERY_STRING", "")))
print("</body>")
print("</html>")
