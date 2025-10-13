# 🌐 Webserv — Lightweight C++ HTTP Server

A fully custom HTTP/1.1 web server written in **C++**, designed to mimic the behavior of **NGINX** while following the **42 School webserv project** specifications.

This server supports multiple simultaneous connections, dynamic CGI execution, configuration files with NGINX-like syntax, and full HTTP method handling — all built from scratch using non-blocking sockets and the **epoll** event system.

---

## 🚀 Features

### ⚙️ Core HTTP Functionality
- Full **HTTP/1.1** compliance
- Persistent **keep-alive** connections
- Handles **GET**, **POST**, and **DELETE** requests
- Configurable **error pages** and **index files**
- Custom **HTTP headers** and MIME type handling

### 🌲 File & Directory Serving
- Serves **static files** directly from disk
- Supports **autoindex** (directory listing)
- File **uploading** to configurable directories
- Proper **MIME type detection**

### 🧩 CGI (Common Gateway Interface)
- Executes external scripts (Python, Bash, etc.)
- Configurable interpreters via `cgi` directive:
  ```nginx
  cgi /usr/bin/python3 py /usr/bin/bash sh;
