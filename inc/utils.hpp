#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstddef>
#include <map>
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include "ParseExeption.hpp"

void set_non_blocking(int socket);
std::string parse_uri(std::string & line);
bool is_directory(const std::string &path);
std::string get_location_name(std::string uri);
std::string decode_url(const std::string &url);
std::string get_content_type(const std::string &filename);
int find_closing_bracket(int &brackets, std::string &line);
std::string get_value_directive(std::string &line, size_t d);
std::string generate_directory_listing(const std::string &directory_path);
int find_opening_bracket(std::ifstream &file, std::string &line, size_t p, size_t &err_line);
std::string generate_directory_listing(const std::string &directory_path, std::string &uri);
void    launch_parse_exception(const std::string & line, const std::string & error_msg, size_t err_line);

#endif