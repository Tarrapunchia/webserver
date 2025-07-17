#include "ServerMan.hpp"
#include "Server.hpp"
#include <cstddef>
#include <signal.h>
#include <sstream>
#include <string>
#include <vector>

ServerMan::ServerMan(char *file)
: first(true)
{
	epoll_fd = -1;
	size_t err_line = 1;
	size_t c;
	std::string line;
	std::ifstream fileCont(file);
	if (!fileCont.is_open())
		throw ParseException("Invalid configuration file!");
	while (std::getline(fileCont, line))
	{
		if ((c = line.find_first_not_of(" \t\n\v\f\r")) != std::string::npos)
		{
			if (line.find_first_not_of(" \t\n\v\f\r", c + 6) == std::string::npos
				&& line.substr(c, 6) == "server")
			{
				Server server(fileCont, line, err_line);
				servers.push_back(server);
			}
			else
			{
				launch_parse_exception(line, "Wrong syntax in configuration file", err_line);
			}
		}
		err_line++;
	}
	if (servers.size() == 0)
		throw ParseException("Empty file!");
	check_hostnames();
}

void ServerMan::check_hostnames()
{
	std::string ipport;
	std::string name;
	// std::vector<std::string> ipports;
	// std::map<std::string, std::vector<Server>> active_servers;  

	// for (size_t i = 0; i < servers.size(); i++) {
	// 	ipport = servers[i].get_ipport();
	// 	name = servers[i].get_name();
	// 	ipports.push_back(ipport);
	// 	active_servers[ipport].push_back(servers[i]);
	// }
	// for (size_t i = 0; i < ipports[i].size(); i++) {
	// 	active_servers[ipports[i]];
	// }
	size_t j = 0;
	for (size_t i = 0; i < servers.size(); i++) 
	{
		ipport = servers[i].get_ipport();
		name = servers[i].get_name();
		j = i + 1;
		while (j < servers.size())
		{
			if (ipport == servers[j].get_ipport())
			{
				std::map<std::string, Server> map_serv = servers[i].subserver;
				if (map_serv.find(servers[j].get_name()) != map_serv.end() 
					|| servers[i].get_name() == servers[j].get_name())
					throw ParseException("Wrong servers configuration!");
				servers[i].subserver.insert(std::pair<std::string, Server>(servers[j].get_name(), servers[j]));
				std::vector<Server>::iterator it_j = servers.begin() + j;
				servers.erase(it_j);
			}
			else
				j++;
		}
	}
}

void ServerMan::ctrl_c(int sig)
{
	(void)sig;
	throw ParseException(" Execution interrupted!");
}

void ServerMan::add_servsock_to_epoll(Server &serv)
{
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN; 
    ev.data.fd = serv.get_socket();
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serv.get_socket(), &ev) == -1) 
		throw ParseException("Failed to add server socket to epoll!");
	serv_sockets.insert(serv.get_socket());
}

void ServerMan::add_clientsock_to_epoll(int socket)
{
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN;
	ev.data.fd = socket;
	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket, &ev) == -1) 
	{
		std::cerr << "Failed to add client socket to epoll\n";
		close(socket);
	}
}

void ServerMan::create_epoll()
{
	if ((epoll_fd = epoll_create1(0)) == -1)
		throw ParseException("Failed: Could not create epoll!");
}

void ServerMan::server_init()
{
	create_epoll();
	std::vector<Server>::iterator it = servers.begin();
	while (it != servers.end())
	{
		it->create_socket();
		add_servsock_to_epoll(*it);
		it++;
	}
	handle_server_loop();
}

// void ServerMan::close_client_connection(int socket)
// {
// 	std::cout << "Connection closed!\n";
// 	client_buffers.erase(socket);
// 	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket, NULL);
// 	close(socket);
// }

// bool ServerMan::parse_request(const char *buffer, int client_socket, int server)
// {
// 	std::string line(buffer);
// 	if (line.find("\r\n\r\n") == line.npos)
// 	{
//     	return (false);
// 	}
// 	std::vector<Server>::iterator it = servers.begin();
// 	while (it != servers.end())
// 	{
// 		if (it->get_socket() == server)
// 			break;
// 		it++;
// 	}
// 	it->serve(line, client_socket);
//     return (true);
// }

// void ServerMan::recive_request(int client_socket, int server)
// {
// 	char buffer[4096];
	
// 	memset(buffer, 0, sizeof(buffer));
// 	int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
// 	if (bytes_received <= 0) 
// 		close_client_connection(client_socket);
// 	else
// 	{
// 		client_buffers[client_socket] += std::string(buffer, bytes_received);
// 		std::cout << "Received request:\n" << client_buffers[client_socket] << std::endl;
// 		if (parse_request(client_buffers[client_socket].c_str(), client_socket, server))
// 			close_client_connection(client_socket);
// 	}
// }

void ServerMan::manage_client_connection(int socket)
{
	int client_socket = accept(socket, NULL, NULL);
	if (client_socket == -1) 
		std::cerr << "Accept failed\n";
	std::cout << "Connection accepted\n";
	set_non_blocking(client_socket);
	add_clientsock_to_epoll(client_socket);
}

void ServerMan::handle_server_loop()
{
	signal(SIGINT, &ctrl_c);
	struct epoll_event events[1024];
	int server = 0;
	while (true)
	{
		int nfds = epoll_wait(epoll_fd, events, 1024, -1);
		if (nfds == -1)
			throw ParseException("epoll_wait failed!");
		int n = 0;
		while (n < nfds)
		{
			int fd = events[n].data.fd;
			if (serv_sockets.count(fd))
			{
				server = fd;
				manage_client_connection(fd);
			}
			else if (events[n].events & EPOLLIN)
			{
				for (size_t i = 0; i < servers.size(); i++) {
					if (servers[i].get_socket() == server)
					{
						servers[i].recive_request(events[n].data.fd, epoll_fd);
						break;
					}
					std::cout << "N: " << i << " : ";
					std::cout << server << std::endl;
				}
			}
			n++;
		}
	}
}

ServerMan::~ServerMan()
{
	if (epoll_fd != -1)
		close(epoll_fd);
};
