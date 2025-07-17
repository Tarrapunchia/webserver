#ifndef PARSEEXEPTION_HPP
#define PARSEEXEPTION_HPP

#include <string>

class ParseException : public std::exception
{
	private:
		std::string message;
		std::string cat;
	public:
		ParseException();
		
		ParseException(const std::string& msg);
		ParseException(const ParseException& other);
		ParseException& operator=(const ParseException& other);
		virtual ~ParseException() throw();
		virtual const char* what() const throw();
};

#endif
