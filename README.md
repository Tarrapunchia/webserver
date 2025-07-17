# Webserv

## Overview  
**Webserv** is a non-blocking, single-process HTTP/1.1 server written in C++98.  
It handles static file serving, file uploads, and CGI execution, using a customizable configuration file.

## Build  
```bash
# Build the project
make
```

## Usage  
```bash
./webserv <configuration_file>
```  
- If no configuration file is provided, a default path is used.  
- The server listens on ports and hosts defined in the config.

## Configuration File  
The configuration file uses an NGINX-inspired syntax to define one or more `server` blocks. Key directives:

- **listen** `host:port`  
- **server_name** `name1 [name2 ...]`  
- **error_page** `code path`  
- **client_max_body_size** `bytes`  
- **location** `path { ... }`  
  - **root** `directory`  
  - **methods** `GET [POST DELETE]`  
  - **index** `file`  
  - **autoindex** `on|off`  
  - **return** `status URL` (redirection)  
  - **cgi_pass** `extension /path/to/cgi`  
  - **upload_dir** `directory`

### Example snippet  
```nginx
server {
  listen 127.0.0.1:8080;
  server_name localhost;

  error_page 404 /errors/404.html;
  client_max_body_size 1000000;

  location / {
    root /var/www/html;
    index index.html;
    autoindex off;
  }

  location /upload {
    methods GET POST;
    upload_dir /var/www/uploads;
  }

  location /php {
    cgi_pass .php /usr/bin/php-cgi;
  }
}
```

## Requirements  
- **Language:** C++98 (`-std=c++98`)  
- **Compiler:** `c++` or `g++`  
- **Flags:** `-Wall -Wextra -Werror`  
- **Allowed syscalls:** `socket`, `bind`, `listen`, `accept`, `recv`, `send`, `poll`/`select`/`epoll`/`kqueue`, `fork` (CGI only), file I/O, process control  
- **Forbidden:** external or Boost libraries.

## Behavior & Constraints  
- Single non-blocking event loop (one `poll()` or equivalent) for all sockets.  
- No direct blocking reads/writes outside the event loop.  
- Accurate HTTP response codes; default error pages if not provided.  
- Support for GET, POST, DELETE methods.  
- Fully static file serving and file uploads.  
- CGI execution with correct environment and chunked request handling.  
- Resilient under stress; must never crash.

## macOS Specific  
- Use `fcntl()` with `O_NONBLOCK` and `FD_CLOEXEC` to emulate non-blocking I/O.

## Testing  
- Test with web browsers, `telnet`, and compare behavior with NGINX.  
- Automated stress tests recommended (e.g., using Python, Go).

---

*This README covers the mandatory requirements for the Webserv project.*