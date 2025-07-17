#include "Server.hpp"
#include "Location.hpp"
#include <cctype>
#include <climits>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <ios>
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <ctime>
#include <unistd.h>
#include <sys/wait.h>

void Server::parse_directive(std::ifstream &file, std::string &line, int &brackets, size_t & err_line)
{
	size_t c = line.find_first_not_of(" \t\n\v\f\r");
	if (c == std::string::npos || line[c] == '#' || line[c] == '}')
		return;
	size_t d = line.find_first_of(" \t\n\v\f\r", c);
	std::string directive = line.substr(c, d - c);
	if (directive == "root")
	root = get_value_directive(line, d);
	else if (directive == "listen")
	listenip = get_value_directive(line, d);
	else if (directive == "index")
	index = get_value_directive(line, d);
	else if (directive == "client_max_body_size")
	{
		char *end = NULL;
		long size = strtol(get_value_directive(line, d).c_str(), &end, 10);
		if ((*end && *(end + 1)) || size < 0)
			throw ParseException("Not valid body_size in configuration file!");
		switch (std::tolower(*end)) {
			case ('k'):
				size *= 1e4;
				break;
			case ('m'):
				size *= 1e6;
				break;
			case ('g'):
				size *= 1e9;
				break;
			case (0):
				break;
			default:
				throw ParseException("Not valid body_size in configuration file!");
				break;
		}
		max_body_size = size;
	}
	else if (directive == "server_name")
		server_name = get_value_directive(line, d);
	else if (directive == "location")
	{
			Location location(file, line, err_line);
			locations.push_back(location);
			brackets++;
	}
	else
		throw ParseException("Not valid directive in configuration file!");
}

void Server::get_port_ip()
{
	size_t c = listenip.find_first_of(":");
	char *end = NULL;
	if (c == std::string::npos)
	{
		port = strtol(listenip.c_str(), &end, 10);
		listenip = "";
		return ;
	}
	c += 1;
	size_t d = listenip.find_first_of(";", c);
	port = strtol(listenip.substr(c, d - c).c_str(), &end, 10);
	if (*end != '\0')
		throw ParseException("Not valid port in configuration file!");
	listenip = listenip.substr(0, c - 1);
}

Server::Server(std::ifstream &file, std::string &line, size_t &err_line)
{
	server_socket = -1;
	max_body_size = -1;
	size_t c = line.find_first_of("server");
	c = line.find_first_of(" \t\n\v\f\r", c);
	if (!find_opening_bracket(file, line, c, err_line))
		launch_parse_exception(line,
				"Mismatched bracket in configuration file",
				err_line);
		// throw ParseException("Mismatched bracket in configuration file!");
	int brackets = 0;
	while(std::getline(file, line))
	{
		parse_directive(file, line, brackets, err_line);
		err_line++;
		if (find_closing_bracket(brackets, line))
			break;
		brackets = 0;
	}
	if (brackets == 0)
		launch_parse_exception(line,
			"Mismatched bracket in configuration file",
			err_line);
	// throw ParseException("Mismatched bracket in configuration file!");
	if (!listenip.size() || !root.size())
		throw ParseException("Incomplete configuration file!");
	max_body_size = (max_body_size == -1)? 1e6: max_body_size;
	get_port_ip();
}

void Server::prepare_sockddr()
{
	memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
	if (listenip.empty())
    	address.sin_addr.s_addr = inet_addr("0.0.0.0");
	else
		address.sin_addr.s_addr = inet_addr(listenip.c_str());
    address.sin_port = htons(port);
}

void Server::create_socket()
{
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1)
       throw ParseException("Error creating socket!");
	int opt = 1;
	// Set the SO_REUSEADDR socket option level to 1
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	prepare_sockddr();
	if (bind(server_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
		throw ParseException("\033[31mBind failed!\033[0m");
	if (listen(server_socket, SOMAXCONN) < 0)
		throw ParseException("Listen failed!");
	if (!server_name.empty())
		std::cout << "\033[33mServer: " + server_name;
	else if (!listenip.empty())
		std::cout << "\033[33mServer ip: " + listenip;
	else
	{
		char name[HOST_NAME_MAX] = {};
		gethostname(name, HOST_NAME_MAX);
		std::cout << "\033[33mHOSTNAME: " << name;
	}
	std::cout << " is listening on port " << port << "...\033[0m\n";
	set_non_blocking(server_socket);
}

void	Server::send_response(int code, char *succeed, const char* type, const std::string & body, int socket, std::string cookie)
{
	std::ostringstream response;
    response << "HTTP/1.1 " << code <<" " << succeed << "\r\n"
             << "Content-Type: " << type << "\r\n";
	if (cookie == "solid-snake")
		response << "Set-Cookie: " << cookie.c_str() << "=true; Max-Age=60\r\n";
    response << "Connection: close\r\n\r\n";
	send(socket, response.str().c_str(), response.str().size(), 0);
	std::cout << response.str();
	response << body;
	send(socket, body.c_str(), body.size(), 0);
}

Server  *Server::check_server(int client_socket, Server **s)
{
	*s = this;
	try {
		*s = &subserver.at(REQ.host);
	} catch (std::out_of_range & err) {};

	return (*s);
}

LOCATION_ERR Server::check_location(std::string &path, std::string method, size_t &location_index, int client_socket)
{
	Server *s = NULL;
	check_server(client_socket, &s);

	for (location_index = 0; location_index < s->locations.size(); location_index++) {
		const std::string& loc_name = s->locations[location_index].get_name();
		if (path.compare(0, loc_name.size(), loc_name) == 0 &&
			(path.size() == loc_name.size() || path[loc_name.size()] == '/'))
			break;
	}
	if (location_index == locations.size())
		return NO_LOCATION;
	if (location_index < locations.size())
	{
		for (size_t i = 0; i < s->locations[location_index].get_allowed_methods().size(); i++)
		{	
			if (s->locations[location_index].get_allowed_methods()[i] == method)
				return (OK);
		}
	}
	return (NO_METHOD);
}

void Server::serve_html_form(int client_socket, std::string &file_name, std::string cookie)
{
    std::ostringstream response;
	std::string path = decode_url(file_name);
	std::ifstream file;
	file_name = path;
	file.open(file_name.c_str(), std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "Error: Could not open " << file_name << std::endl;
		send_error_page(client_socket, "404.html", "GET");
        return;
    }
	std::string type = get_content_type(file_name);
	std::string line;
	while (std::getline(file, line))
		response << line << "\n";
	file.close();
	send_response(200, (char *)"OK", type.c_str(), response.str(), client_socket, cookie);
}

void Server::send_error_page(int client_socket, std::string error_page, std::string method)
{
	size_t location_index = 0;
	std::ifstream file;
	std::ostringstream error_response;
	std::string path = "/" + error_page;
	if (check_location(path, method, location_index, client_socket) == OK)
	{
		Server *s = NULL;
		check_server(client_socket, &s);
		file.open((s->locations[location_index].get_root() + path).c_str());
		if (!file.is_open())
			file.open(("./error_pages/" + error_page).c_str());
		std::string line;
		while (std::getline(file, line))
			error_response << line << "\n";
		file.close();
	}
	else
	{
		file.open(("./error_pages/" + error_page).c_str());
		std::string line;
		while (std::getline(file, line))
			error_response << line << "\n";
		file.close();
	}
	std::string code = error_page.substr(0, 3);
	send_response(std::strtol(code.c_str(), NULL, 10), (char *)"KO", "text/html", error_response.str(), client_socket, "");
	return;
}

bool directory_exists(const std::string& path) {
    struct stat info;
    return (stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFDIR));
}

bool create_directory(const std::string& path) {
    return (mkdir(path.c_str(), 0755) == 0);
}

bool Server::handle_post(int client_socket, std::string & body)
{
	if (max_body_size != 0 && REQ.length > max_body_size)
	{
		send_response(413, (char *)"KO", "text/plain", "Request Entity Too Large.", client_socket, "");
		return (false);
	}
    Request & req = REQ;
    (void)body;
    std::string decoded_uri = decode_url(REQ.uri);
	
    size_t location_index = 0;
	
    LOCATION_ERR res = check_location(decoded_uri, "POST", location_index, client_socket);
    if (res == NO_METHOD) {
		send_error_page(client_socket, "405.html", "GET");
        return true;
    }
	
    if (res == NO_LOCATION) {
		std::cerr << "No Location Found." << std::endl;
        send_error_page(client_socket, "404.html", "GET");
        return true;
    }
    
	Server *s = NULL;
	check_server(client_socket, &s);
	if (s->locations[location_index].get_cgi() == "on")
	{
		execute_cgi(decoded_uri, "POST", client_socket);
		return true;
	}

    // Gestione dei dati multipart
	std::string content;
    if (req.content_type.find("multipart/form-data") != std::string::npos) {
        // Cerca l'inizio del contenuto (dopo gli header del form-data)
        size_t beginning = req.body.find("\r\n\r\n");
        if (beginning != std::string::npos) {
            content = req.body.substr(beginning + 4);
        } else {
            // Non è stato trovato l'inizio del contenuto, salva tutto il body
            content = req.body;
        }
    } else {
        // Non è multipart, salva tutto il body
        content = req.body;
    }
    send_response(200, (char*)"OK", "text/plain", content.substr(0, content.find("\r\n")), client_socket, "");
    return true;
}

std::string	select_language(const std::string & path)
{
	if ((path.rfind(".py")!=std::string::npos))
		return (PYTHON);
	if ((path.rfind(".php")!=std::string::npos))
		return (PHP);
	if ((path.rfind(".js")!=std::string::npos))
		return (JAVASCRIPT);
	if ((path.rfind(".sh")!=std::string::npos))
		return (BASH);
	return "";
}

void Server::execute_cgi(std::string path, std::string method, int client_socket)
{
	std::string abs_path = "." + path;

	std::string exec_prog;                 // es. /usr/bin/php-cgi
	if (access(abs_path.c_str(), X_OK) == 0) {   // lo script è già eseguibile (has she‑bang)
		exec_prog = abs_path;
	} else {
		/* fallback per estensione */
		exec_prog = select_language(abs_path);
		if (exec_prog.empty()) {
			send_response(500,(char*)"KO","text/plain","CGI: unknown extension",client_socket, "");
			return;
		}
	}
	std::string dir_path = "./files";
	if (!directory_exists(dir_path)) {
		if (!create_directory(dir_path)) {
			send_response(500, (char*)"KO", "text/plain", "Internal Server Error: Cannot create files directory", client_socket, "");
			return ;
		}
	}

	/* ---------- crea le pipe ---------------------------------------------------------------- */
	int inPipe[2], outPipe[2];
	if (pipe(inPipe)==-1 || pipe(outPipe)==-1) {
		send_response(500,(char*)"KO","text/plain","pipe error",client_socket, "");
		return;
	}

	/* ---------- env secondo CGI spec -------------------------------------------------------- */
	std::vector<std::string> envv;
	envv.push_back("GATEWAY_INTERFACE=CGI/1.1");
	envv.push_back("SERVER_SOFTWARE=WebServer/1.0");
	envv.push_back("SERVER_PROTOCOL="+REQ.protocol);
	envv.push_back("REQUEST_METHOD="+method);

	std::string host = server_name.empty()? "localhost":server_name;
	envv.push_back("SERVER_NAME="+host);
	envv.push_back("SCRIPT_FILENAME="+abs_path);
	envv.push_back("SCRIPT_NAME="+REQ.uri);

	envv.push_back("REMOTE_ADDR=" + std::string(inet_ntoa(address.sin_addr)));

	if (method=="POST") {
		envv.push_back("CONTENT_TYPE="+ REQ.content_type);
		std::stringstream ss;
		ss << "CONTENT_LENGTH=" << REQ.length;
		envv.push_back(ss.str());
	}
	if (REQ.cookie.find("solid-snake") != std::string::npos)
		envv.push_back("COOKIES=solid-snake");

	std::vector<char*> envp;
	for (std::size_t i=0;i<envv.size();++i)
		envp.push_back(strdup(envv[i].c_str()));
	envp.push_back(NULL);

	std::vector<char*> argv;
	argv.push_back(strdup(exec_prog.c_str()));
	if (exec_prog != abs_path)
		argv.push_back(strdup(abs_path.c_str()));
	argv.push_back(NULL);

	pid_t pid = fork();
	if (pid==-1) {
		send_response(500,(char*)"KO","text/plain","fork error",client_socket, "");
		return;
	}
	if (pid==0) {
		dup2(inPipe[0], STDIN_FILENO);   close(inPipe[1]);
		dup2(outPipe[1],STDOUT_FILENO);  close(outPipe[0]);
		execve(exec_prog.c_str(), argv.data(), envp.data());
		perror("execve");
		std::cerr << "execve failed with errno: " << errno << " (" << strerror(errno) << ")" << std::endl;
		std::cerr << exec_prog.c_str();
		_exit(1);
	}
	close(inPipe[0]);   
	close(outPipe[1]);

	if (method=="POST") {
		const std::string& body = REQ.body;
		ssize_t w = 0; while (w < (ssize_t)body.size())
		w += write(inPipe[1], body.data() + w, body.size()-w);
	}
	close(inPipe[1]);

	std::string cgi_out;
	char buf[4096];
	ssize_t n;
	while ((n = read(outPipe[0], buf, sizeof(buf))) > 0)
		cgi_out.append(buf, n);
	close(outPipe[0]);

	int status;
	waitpid(pid,&status,0);

	for (size_t i = 0; i < envp.size(); i++) {
		free(envp[i]);
	}
	for (size_t i = 0; i < argv.size(); i++) {
		free(argv[i]);
	}

	if (!WIFEXITED(status) || WEXITSTATUS(status)!=0){
		send_response(500,(char*)"KO","text/plain","CGI exited with error",client_socket, "");
		return;
	}

	// ricordarsi di levare header da cgi (nello script) in modo da risparmiarsi un doppio header
	send_response(200, (char*)"OK", "text/html", cgi_out.c_str(), client_socket, "");
}

void Server::manage_get(std::string &line, int client_socket)
{
	std::cerr << line;
	size_t location_index = 0;
	LOCATION_ERR res = check_location(REQ.uri, "GET", location_index, client_socket);
	Server *s = NULL;
	check_server(client_socket, &s);
	if (res == NO_LOCATION)
	{
		std::string path;
		if (REQ.uri == "/")
		{
			if (REQ.cookie == "solid-snake=true")
			{
				path = "./home/solidsnake.html";
				serve_html_form(client_socket, path, "");
				return;
			}
			path = s->get_root() + REQ.uri + s->get_index();
			serve_html_form(client_socket, path, "solid-snake");
			return;
		}
		else
			path = s->get_root() + REQ.uri;
		serve_html_form(client_socket, path, "");
		return ;
	}
	else if (res == NO_METHOD)
		send_error_page(client_socket, "405.html", "GET");
	else
	{
		std::string path;
		std::string root = s->locations[location_index].get_root();
		if (root.empty())
		{
			std::string alias = s->locations[location_index].get_alias();
			if (alias.empty())
				path = get_root() + REQ.uri;
			else
				path = alias;
		}
		else
			path = root + REQ.uri;
		if (is_directory(path) && (s->locations[location_index].get_autoindex() == "on"))
		{
			std::string body = generate_directory_listing(path, REQ.uri);
			send_response(200, (char *)"Ok", "text/html", body, client_socket, "");
		}
		else if (s->locations[location_index].get_cgi() == "on")
		{
			size_t fnd = path.find("./");
			if (fnd != std::string::npos)
				path = path.substr(fnd + 2);
			execute_cgi(path, "get", client_socket);
		}
		else
			serve_html_form(client_socket, path, "");
	}
}

void Server::handle_delete(std::string &line, int client_socket)
{
	std::cerr << line;
	size_t location_index = 0;
	LOCATION_ERR res = check_location(REQ.uri, "DELETE", location_index, client_socket);
	Server *s = NULL;
	check_server(client_socket, &s);
	if (res == NO_LOCATION)
	{
		std::string path;
		std::ifstream file;
		path = s->get_root() + REQ.uri;
		file.open(path.c_str());
		if (!is_directory(path)) 
		{
			if (file.is_open())
				send_response(403, (char *)"Forbidden", "text/html", "<h1>403 Forbidden</h1>", client_socket, "");
			else
				send_response(404, (char *)"Not Found", "text/html", "<h1>404 Not Found</h1>", client_socket, "");
		} 
		else
			send_response(403, (char *)"Forbidden", "text/html", "<h1>403 Forbidden</h1>", client_socket, "");
		return ;
	}
	else if (res == NO_METHOD)
		send_response(405, (char *)"Not Allowed", "text/html", "<h1>405 Not Allowed</h1>", client_socket, "");
	else
	{
		std::string path = s->locations[location_index].get_root() + REQ.uri;
		if (is_directory(path))
			send_response(403, (char *)"Forbidden", "text/html", "<h1>403 Forbidden</h1>", client_socket, "");
		else
		{
			std::ifstream file;
			file.open(path.c_str());
			if (!file.is_open())
				send_response(404, (char *)"Not Found", "text/html", "<h1>404 Not Found</h1>", client_socket, "");
			else
			{
				file.close();
				if (std::remove(path.c_str()) == 0)
					send_response(200, (char *)"Ok", "text/html", "<h1>File successfully deleted</h1>", client_socket, "");
			}
		}
	}
}

void populate_req(Request &req)
{
	std::string line;
	
	// setto header
	size_t header_end = req.header.find("\r\n\r\n");
	if (header_end != std::string::npos) {
		req.body = req.header.substr(header_end + 4);
		req.body_size = req.body.size();
		req.header = req.header.substr(0, header_end);
	}
	
	std::istringstream stream(req.header);
	// Prima riga: metodo, URI, protocol
	if (std::getline(stream, line)) {
		std::istringstream first_line(line);
		first_line >> req.method >> req.uri >> req.protocol;
	}

	// Header fields
	while (std::getline(stream, line)) {
		if (line == "\r" || line.empty())
			break;

		if (line.find("Host:") == 0)
		{
			req.host = line.substr(6); // Salta "Host: "
			req.host = req.host.substr(0, req.host.find(":"));
		}
		else if (line.find("User-Agent:") == 0)
			req.user_agent = line.substr(12);
		else if (line.find("Accept:") == 0)
			req.accept = line.substr(8);
		else if (line.find("Cookie:") == 0)
			req.cookie = line.substr(8).c_str();
		else if (line.find("Content-Length:") == 0)
			req.length = std::atoi(line.substr(16).c_str());
		else if (line.find("Content-Type:") == 0)
		{
			req.content_type = line.substr(14);
			if (req.content_type.find("multipart/form-data") != std::string::npos)
			{
				std::size_t boundary_pos = req.content_type.find("boundary=");
				if (boundary_pos != std::string::npos)
				{
					req.boundary = req.content_type.substr(boundary_pos + 9, req.content_type.find("\r\n"));
					req.boundary = req.boundary.substr(0, req.boundary.size() > 0? - 1: 0);
				}
			}
		}
	}
	req.active = true;
}

bool Server::parse_request(const char *buffer, int client_socket)
{
	std::string line(buffer);
	if (line.find("\r\n\r\n") == line.npos)
	{
    	return (false);
	}
	if (REQ.active == false)
	{
		populate_req(REQ);
		line = line.substr(REQ.header.length() + 4);
	}
	if (REQ.method == "POST")
		REQ.phase = BODY;
    return (serve(line, client_socket));
}

void Server::close_client_connection(int socket, int epoll_fd)
{
	std::cout << "Connection closed!\n";
	client_buffers.erase(socket);
	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket, NULL);
	close(socket);
}

void Server::recive_request(int client_socket, int epoll_fd)
{
    char buffer[1024];
    
    memset(buffer, 0, sizeof(buffer));
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
	if (bytes_received <= 0)
    {
        close_client_connection(client_socket, epoll_fd);
    }
	else
    {
        if (REQ.phase == HEADER)
        {
			REQ.header += std::string(buffer, bytes_received);
			if (parse_request(REQ.header.c_str(), client_socket)) {
				if (REQ.method == "GET") {
					close_client_connection(client_socket, epoll_fd);
				}
			} else if (REQ.method == "POST") {
				// Attiva fase BODY solo dopo che HEADERS sono completi
				if (REQ.body.size() >= REQ.length) {
					handle_post(client_socket, REQ.body);
					close_client_connection(client_socket, epoll_fd);
				} else {
					REQ.phase = BODY; // fondamentale!
				}
			}
        }
        else if (REQ.phase == BODY)
        {
            REQ.body += std::string(buffer, bytes_received);
            REQ.body_size += bytes_received;
            if (REQ.body_size >= REQ.length) // ho ricevuto tt il corpo
            {
                handle_post(client_socket, REQ.body);
                close_client_connection(client_socket, epoll_fd);
            }
        }
    }
}

bool Server::serve(std::string & line, int client_socket)
{
	if (REQ.method == "GET")
	{
		manage_get(line, client_socket);
		return (true);
	}
	else if (REQ.method == "POST")
		return (false);
	return (true);
}

Server::~Server(){
	if (server_socket != -1)
	close(server_socket);
};

// getters
std::string Server::get_ipport() const {
	std::stringstream ss;
	ss << port;
	return (listenip + ss.str());
}

int Server::get_socket(){ return server_socket; }
std::string Server::get_name() const { return (server_name); }
std::string Server::get_root() const { return (root); }
std::string Server::get_index() const { return (index); }
