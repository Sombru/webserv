#!/bin/bash

PORT=8082
HOST=localhost

echo "[1] Starting silent client..."
# Connects and stays idle without sending anything
( nc "$HOST" "$PORT" > /dev/null & ) 
SILENT_PID=$!

# Wait to ensure the silent client is connected
sleep 2

echo "[2] Sending GET request from second client..."
RESPONSE=$(echo -e "GET / HTTP/1.1\r\nHost: $HOST\r\n\r\n" | nc "$HOST" "$PORT")

echo "[3] Server response:"
echo "----------------------"
echo "$RESPONSE"
echo "----------------------"

# Kill the silent client (if still running)
if ps -p $SILENT_PID > /dev/null; then
    kill $SILENT_PID
    echo "[4] Silent client terminated."
fi