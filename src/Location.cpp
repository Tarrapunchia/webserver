#include "Location.hpp"
#include <cstddef>
#include <stdlib.h>

void Location::set_allowed_methods(std::string &line, size_t d)
{
	size_t c = line.find_first_not_of(" \t\n\v\f\r", d);
	if (c == std::string::npos || (line[c] == ';' && methods.size() == 0))
		// launch_parse_exception(line, "No value found for directive in location", err_line)
		throw ParseException("No value found for directive in location!");
	d = line.find_first_of(" \t\n\v\f\r;", c);
	std::string method = line.substr(c, d - c);
	if (method == "GET" || method == "POST" || method == "DELETE")
		methods.push_back(method);
	else
		throw ParseException("Invalid method in location!");
	if ((c = line.find_first_not_of(" \t\n\v\f\r", d)) == std::string::npos)
		throw ParseException("No semicolon ';' found after directive in configuration file!");
	else if (line[c] == ';')
	{
		if (c != d)
			throw ParseException("Found space before semicolon ';' in configuration file");
		else if (line.find_first_not_of(" \t\n\v\f\r", c + 1) != std::string::npos)
			throw ParseException("Unexpected content following the semicolon ';' in configuration file!");
		else
			return;
	}
	else
		set_allowed_methods(line, c);
}

void Location::parse_directive_loc(std::ifstream &file, std::string &line)
{
	(void)file;
	size_t c = line.find_first_not_of(" \t\n\v\f\r");
	if (c == std::string::npos || line[c] == '#' || line[c] == '}')
		return;
	size_t d = line.find_first_of(" \t\n\v\f\r", c);
	std::string directive = line.substr(c, d - c);
	if (directive == "root")
		root = get_value_directive(line, d);
	else if (directive == "alias")
		alias = get_value_directive(line, d);
	else if (directive == "autoindex")
		autoindex = get_value_directive(line, d);
	else if (directive == "cgi")
		cgi = get_value_directive(line, d);
	else if (directive == "max_body_size")
	{
		char *end = NULL;
		long size = strtol(get_value_directive(line, d).c_str(), &end, 10);
		if (*end != '\0')
			throw ParseException("Not valid body_size in configuration file!");
		max_body_size = size;
	}
	else if (directive == "path_cgi")
		cgi_path = get_value_directive(line, d);
	else if (directive == "allowed_methods")
		set_allowed_methods(line, d);
	else
		throw ParseException("Invalid directive in location!");
}

Location::Location(std::ifstream &file, std::string &line, size_t &err_line)
{
	size_t c = line.find_first_of("location");
	if ((c = line.find_first_not_of(" \t\n\v\f\r", c + 8)) == std::string::npos)
		throw ParseException("No path location specified in configuration file!");
	size_t d = line.find_first_of(" \t\n\v\f\r", c);
	name = line.substr(c, d - c);
	if (!find_opening_bracket(file, line, d, err_line))
		throw ParseException("Mismatched bracket in location directive!");
	int brackets = 0;
	while(std::getline(file, line))
	{
		parse_directive_loc(file, line);
		err_line++;
		if (find_closing_bracket(brackets, line))
			break;
	}
	if (brackets == 0)
		throw ParseException("Mismatched bracket in configuration file!");
}


// getters
long		Location::get_max_body_size() { return max_body_size; }
std::string Location::get_name() { return name; }
std::string Location::get_root() { return root; }
std::string Location::get_alias() { return alias; }
std::string Location::get_index() { return index; }
std::string Location::get_autoindex() { return autoindex; }
std::string Location::get_cgi() { return cgi; }
std::vector<std::string> Location::get_allowed_methods() { return methods; }

// copy constr && copy assignment
Location::Location(const Location &other)
{
	this->max_body_size = other.max_body_size;
	this->name = other.name;
	this->cgi_path = other.cgi_path;
	this->root = other.root;
	this->alias = other.alias;
	this->methods = other.methods;
	this->autoindex = other.autoindex;
	this->cgi = other.cgi;
}

Location &Location::operator=(const Location &other)
{
	if (this != &other)
	{
		this->max_body_size = other.max_body_size;
		this->name = other.name;
		this->cgi_path = other.cgi_path;
		this->root = other.root;
		this->alias = other.alias;
		this->methods = other.methods;
		this->autoindex = other.autoindex;
		this->cgi = other.cgi;
	}
	return *this;
}

Location::~Location(){}
