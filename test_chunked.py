#!/usr/bin/env python3
"""
Test script for chunked transfer encoding support in webserv
"""

import socket
import time

def test_chunked_request():
    """Send a chunked POST request to the server"""
    
    # Connect to the server
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(5)
    
    try:
        sock.connect(('127.0.0.3', 8080))
        
        # Prepare chunked POST request
        headers = (
            "POST /test HTTP/1.1\r\n"
            "Host: 127.0.0.3:8080\r\n"
            "Transfer-Encoding: chunked\r\n"
            "Connection: close\r\n"
            "\r\n"
        )
        
        # Send headers
        sock.send(headers.encode())
        time.sleep(0.1)
        
        # Send chunked data
        chunk1 = "Hello "
        chunk2 = "World!"
        chunk3 = " This is a test of chunked encoding."
        
        # Chunk 1
        chunk1_header = hex(len(chunk1))[2:] + "\r\n"
        sock.send(chunk1_header.encode())
        sock.send(chunk1.encode())
        sock.send(b"\r\n")
        time.sleep(0.1)
        
        # Chunk 2
        chunk2_header = hex(len(chunk2))[2:] + "\r\n"
        sock.send(chunk2_header.encode())
        sock.send(chunk2.encode())
        sock.send(b"\r\n")
        time.sleep(0.1)
        
        # Chunk 3
        chunk3_header = hex(len(chunk3))[2:] + "\r\n"
        sock.send(chunk3_header.encode())
        sock.send(chunk3.encode())
        sock.send(b"\r\n")
        time.sleep(0.1)
        
        # Final chunk (size 0)
        sock.send(b"0\r\n\r\n")
        
        # Receive response
        response = b""
        while True:
            try:
                data = sock.recv(1024)
                if not data:
                    break
                response += data
            except socket.timeout:
                break
        
        print("Chunked request sent successfully!")
        print("Response received:")
        print(response.decode())
        
    except Exception as e:
        print(f"Error: {e}")
    finally:
        sock.close()

def test_large_body():
    """Test large body handling"""
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(10)
    
    try:
        sock.connect(('127.0.0.3', 8080))
        
        # Create a large body (1MB)
        large_body = "A" * (1024 * 1024)
        
        headers = (
            "POST /test HTTP/1.1\r\n"
            "Host: 127.0.0.3:8080\r\n"
            f"Content-Length: {len(large_body)}\r\n"
            "Connection: close\r\n"
            "\r\n"
        )
        
        # Send headers
        sock.send(headers.encode())
        
        # Send body in chunks
        chunk_size = 8192
        for i in range(0, len(large_body), chunk_size):
            chunk = large_body[i:i+chunk_size]
            sock.send(chunk.encode())
            time.sleep(0.001)  # Small delay
        
        # Receive response
        response = b""
        while True:
            try:
                data = sock.recv(1024)
                if not data:
                    break
                response += data
            except socket.timeout:
                break
        
        print("Large body request sent successfully!")
        print("Response received:")
        print(response.decode()[:500] + "..." if len(response.decode()) > 500 else response.decode())
        
    except Exception as e:
        print(f"Error: {e}")
    finally:
        sock.close()

if __name__ == "__main__":
    print("Testing chunked encoding and large body support...")
    print("\n1. Testing chunked transfer encoding:")
    test_chunked_request()
    
    print("\n2. Testing large body (1MB):")
    test_large_body()
