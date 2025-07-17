#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "utils.hpp"
#include <vector>

class Location
{
	private:
		std::string name;
		std::string root;
		std::string alias;
		std::string index;
		std::string autoindex;
		std::vector<std::string> methods;
		std::string cgi;
		std::string cgi_path;
		long		max_body_size;
		void parse_directive_loc(std::ifstream &file, std::string &line);
		void set_allowed_methods(std::string &line, size_t d);
	public:
		Location() {};
		Location(std::ifstream &file, std::string &line, size_t &err_line);
		Location(const Location & other);
		Location & operator=(const Location & other);
		
		// getters
		long 		get_max_body_size();
		std::string get_cgi();
		std::string get_name();
		std::string get_root();
		std::string get_alias();
		std::string get_index();
		std::string get_autoindex();
		std::vector<std::string> get_allowed_methods();
		~Location();
};

#endif
