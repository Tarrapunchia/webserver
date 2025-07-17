#include "utils.hpp"
#include <cstddef>
#include <sstream>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ios>

std::string get_value_directive(std::string &line, size_t d)
{
	size_t c = line.find_first_not_of(" \t\n\v\f\r", d);
	if (c == std::string::npos || line[c] == ';')
		throw ParseException("No value found for directive in configuration file!");
	d = line.find_first_of(" \t\n\v\f\r;", c);
	std::string directive = line.substr(c, d - c);
	if ((c = line.find_first_of(";", d)) == std::string::npos || c != d)
		throw ParseException("No semicolon ';' found after directive in configuration file!");
	else
	{
		d = line.find_first_not_of(" \t\n\v\f\r;", c);
		if (d != std::string::npos && line[d] != '#')
			throw ParseException("Unexpected content following the semicolon ';' in configuration file!");
	}
	return directive;
}

int find_closing_bracket(int &brackets, std::string &line)
{
	size_t c;
	if ((c = line.find_first_of('}')) != std::string::npos && brackets == 0)
	{
		if (line.find_first_not_of(" \t\n\v\f\r", c + 1) != std::string::npos)
			throw ParseException("Unexpected content following the semicolon '}' in configuration file!");
		brackets = 1;
		return 1;
	}
	return 0;
}

int find_opening_bracket(std::ifstream &file, std::string &line, size_t p, size_t & err_line)
{
	size_t c;
	c = line.find_first_not_of(" \t\n\v\f\r", p);
	if (c == std::string::npos || line[c] != '{')
	{
		std::getline(file, line);
        err_line++;
		c = line.find_first_not_of(" \t\n\v\f\r");
		if (c == std::string::npos || line[c] != '{')
			return 0;
	}
	return 1;
}

// Set to non_blocking the socket flags
void set_non_blocking(int sockfd) 
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

std::string generate_directory_listing(const std::string &directory_path, std::string &uri)
{
    DIR *dir;
    struct dirent *entry;
    std::stringstream html;

    dir = opendir(directory_path.c_str());
    if (!dir)
	{
        return "<html><body><h1>Cannot open directory</h1></body></html>";
	}
	html << "<html><head><link rel=\"stylesheet\" href=\"/stile.css\"></head><body><ul>";

	std::string dir_path = uri;
    if (dir_path[dir_path.size() - 1] != '/')
        dir_path += '/';
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;

        // skip . and ..
        if (name == "." || name == "..")
            continue;

        html << "<li><a href=\"" << dir_path + name << "\">" << name << "</a></li>";
    }

    closedir(dir);
	html << "<audio id=\"bg-audio\" loop>\n"
		<< "  <source src=\"/solidsnake-intruder.mp3\" type=\"audio/mpeg\">\n"
		<< "  Your browser does not support the audio element.\n"
		<< "</audio>\n"
		<< "<script>\n"
		<< "  document.addEventListener('click', function startAudioOnce() {\n"
		<< "    const audio = document.getElementById('bg-audio');\n"
		<< "    audio.volume = 0.2;\n"
		<< "    audio.play().catch(e => console.log('Autoplay blocked:', e));\n"
		<< "    document.removeEventListener('click', startAudioOnce);\n"
		<< "  });\n"
		<< "</script>\n"
		<< "</ul></body></html>";

    return html.str();
}

std::string get_content_type(const std::string &filename)
{
    size_t dot = filename.find_last_of('.');
    if (dot == std::string::npos)
        return "application/octet-stream";
    std::string ext = filename.substr(dot + 1);
    if (ext == "html") return "text/html";
    if (ext == "css") return "text/css";
    if (ext == "js") return "application/javascript";
    if (ext == "png") return "image/png";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "gif") return "image/gif";
    if (ext == "txt") return "text/plain";
    if (ext == "hpp") return "text/plain";
    if (ext == "cpp") return "text/plain";
    if (ext == "pdf") return "application/pdf";
    if (ext == "mp4") return "video/mp4";
    if (ext == "mp3") return "audio/mpeg";

    return "";
}

std::string decode_url(const std::string &url)
{
    std::string decoded;
    for (size_t i = 0; i < url.size(); ++i)
    {
        if (url[i] == '%')
        {
            if (i + 2 < url.size())
            {
                int val;
                std::istringstream(url.substr(i + 1, 2)) >> std::hex >> val;
                decoded += static_cast<char>(val);
                i += 2;
            }
        }
        else
        {
            decoded += url[i];
        }
    }
    return decoded;
}

bool is_directory(const std::string &path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
        return false;
    return (info.st_mode & S_IFDIR);
}

// std::string parse_uri(std::string & line)
// {
// 	size_t pos = line.find('/');
//     if (pos == std::string::npos)
//         return ("");
// 	std::string substr(line, pos);
// 	pos = substr.find(' ');
// 	substr = substr.substr(0, pos);
// 	std::cout << "URI: " << substr << std::endl;
// 	return (substr);
// }

std::string get_location_name(std::string uri)
{
	size_t pos;
	pos = uri.find_last_of("/");
	if (pos != std::string::npos && pos != 0)
		return uri.substr(0, pos + 1);
	return uri;
}
void    launch_parse_exception(const std::string & line, const std::string & error_msg, size_t err_line)
{
    std::stringstream ss;
    ss << error_msg
    << " at line: "
    << err_line
    << ":\n->\t"
    << line;
    throw ParseException(ss.str());
}