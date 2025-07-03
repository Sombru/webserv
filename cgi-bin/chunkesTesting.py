#!/usr/bin/env python3
import os

print("Content-Type: text/html\n")
print("<h1>Hello from CGI!</h1>")
print("<p>Query: " + os.environ.get("QUERY_STRING", "") + "</p>")
print("<p>: 4\r\n" + "</p>")
print("<p>: Wiki\r\n" + "</p>")
print("<p>: 5\r\n" + "</p>")
print("<p>: pedia\r\n" + "</p>")
print("<p>: 0\r\n" + "</p>")
print("<p>: \r\n" + "</p>")