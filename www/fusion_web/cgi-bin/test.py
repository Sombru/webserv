#!/usr/bin/env python3
import sys
import os

# Read stdin
body = sys.stdin.read()

# Print CGI response
print("Content-Type: text/plain")
print()
print("CGI Python script received:")
print('REQUEST_METHOD=' + os.environ.get('REQUEST_METHOD', ''))
print('QUERY_STRING=' + os.environ.get('QUERY_STRING', ''))
print('CONTENT_LENGTH=' + os.environ.get('CONTENT_LENGTH', ''))
print('CONTENT_TYPE=' + os.environ.get('CONTENT_TYPE', ''))
print()
print('---- BODY START ----')
print(body)
print('---- BODY END ----')
