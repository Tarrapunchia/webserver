#ifndef SERVERMAN_HPP
#define SERVERMAN_HPP

#include "Server.hpp"
#include <set>
#include <map>

class ServerMan
{
	private:
		bool first;
		int epoll_fd;
		std::vector<Server> servers;
		std::set<int> serv_sockets;
		std::map<int, std::string> client_buffers;
		void create_epoll();
		void handle_server_loop();
		static void ctrl_c(int sig);
		void manage_client_connection(int socket);
		void add_servsock_to_epoll(Server &serv);
		void add_clientsock_to_epoll(int socket);
		void recive_request(int client_socket, int server);
		bool parse_request(const char *buffer, int client_socket, int server);
		void close_client_connection(int socket);
		void check_hostnames();
	public:
		void server_init();
		ServerMan(char *file);
		~ServerMan();
};

#endif
