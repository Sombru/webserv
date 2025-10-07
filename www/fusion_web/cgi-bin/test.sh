#!/usr/bin/env bash
# Simple CGI shell script that echoes headers and body
read -r -d '' BODY || true

echo "Content-Type: text/plain"
echo
printf "CGI Shell script received:\n"
printf "REQUEST_METHOD=%s\n" "$REQUEST_METHOD"
printf "QUERY_STRING=%s\n" "$QUERY_STRING"
printf "CONTENT_LENGTH=%s\n" "$CONTENT_LENGTH"
printf "CONTENT_TYPE=%s\n" "$CONTENT_TYPE"

echo
printf "---- BODY START ----\n"
printf "%s\n" "$BODY"
printf "---- BODY END ----\n"
