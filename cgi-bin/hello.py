#!/usr/bin/env python3
import os

print("Content-Type: text/html\n")
print("<h1>Hello from CGI!</h1>")
print("<p>Query: " + os.environ.get("QUERY_STRING", "") + "</p>")