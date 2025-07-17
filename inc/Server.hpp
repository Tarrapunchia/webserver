#ifndef SERVER_HPP
#define SERVER_HPP
#include <cstddef>
#include <fstream>
#include <string>
#define REQ client_buffers[client_socket]
#define PYTHON "/usr/bin/python3"
#define PHP "/usr/bin/php"
#define JAVASCRIPT "/usr/share/javascript"
#define BASH "/usr/bin/bash"
#include "Location.hpp"
#include <cstring>
#include <cstdlib>
#include <ostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <dirent.h>
#include <sys/stat.h>

enum LOCATION_ERR {
	NO_LOCATION = 0,
	NO_METHOD,
	OK
};

enum read_phase {
	HEADER,
	BODY
};

struct Request {
	bool active;
	bool file_created;
	uint32_t body_size;
	read_phase phase;
	std::string cookie;
	std::string request;
	std::string header;
	std::string method;
	std::string uri;
	std::string protocol;
	std::string host;
	std::string user_agent;
	std::string accept;
	uint32_t length;
	std::string content_type;
	std::string boundary;
	std::string body;
	std::string content_form;
	std::string content_name;
	std::string content_filename;
	std::string file_type;
    std::ofstream* file_stream;  // Puntatore a ofstream per la gestione del file

    Request() : active(false), file_created(false), body_size(0), phase(HEADER), file_stream(NULL) {}
    ~Request() { 
        if (file_stream) { 
            file_stream->close(); 
            delete file_stream; 
        } 
    }
};

inline std::ostream& operator<<(std::ostream & os, const Request & req)
{
    os << "Request {\n"
       << "  Header: " << req.header << "\n"
       << "  Method: " << req.method << "\n"
       << "  URI: " << req.uri << "\n"
       << "  Protocol: " << req.protocol << "\n"
       << "  Host: " << req.host << "\n"
       << "  User-Agent: " << req.user_agent << "\n"
       << "  Accept: " << req.accept << "\n"
       << "  Content-Length: " << req.length << "\n"
       << "  Content-Type: " << req.content_type << "\n"
       << "  Boundary: " << req.boundary << "\n"
       << "  Body: " << req.body << "\n"
       << "}";
    return os;
}


class Server
{
	private:
		long 							port;
		long							max_body_size;
		int 							server_socket;
		std::string 					root;
		std::string 					index;
		std::string 					server_name;
		std::string 					listenip;
		std::vector<Location> 			locations;
		struct sockaddr_in 				address;
		void 							get_port_ip();
		void 							prepare_sockddr();
		void 							manage_get(std::string &line, int client_socket);
		void 							handle_delete(std::string &line, int client_socket);
		LOCATION_ERR					check_location(std::string &path, std::string method, size_t &location_index, int client_socket);
		Server *						check_server(int client_socket, Server **s);
		void 							serve_html_form(int client_socket, std::string &file_name, std::string cookie);
		void 							parse_directive(std::ifstream &file, std::string &line, int &brackets, size_t &err_line);
		void 							send_error_page(int client_socket, std::string error_page, std::string method);
		void							send_response(int code, char *succeed, const char* type, const std::string & body, int socket, std::string cookie);
		bool							handle_post(int client_socket, std::string & body);
		void 							close_client_connection(int socket, int epoll_fd);
		bool 							parse_request(const char *buffer, int client_socket);
		void 							execute_cgi(std::string path, std::string method, int client_socket);
		std::map<int, Request> 			client_buffers;
	public:
		std::map<std::string, Server>	subserver;
		void 							recive_request(int client_socket, int epoll_fd);
		bool 							serve(std::string &request, int client_socket);
		void 							create_socket();
		int 							get_socket();
		Location * 						get_location(const std::string & uri);
		std::string						get_ipport() const;
		std::string						get_name() const;
		std::string						get_root() const;
		std::string						get_index() const;
		Server(std::ifstream &file, std::string &line, size_t & err_line);
		~Server();

};

#endif
