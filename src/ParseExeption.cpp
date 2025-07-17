
#include "ParseExeption.hpp"

ParseException::ParseException() : message("Parsing error") {}

ParseException::ParseException(const std::string& msg) : message(msg) {}

ParseException::ParseException(const ParseException& other) : message(other.message) {}

ParseException& ParseException::operator=(const ParseException& other)
{
	if (this != &other)
		message = other.message;
	return *this;
}

ParseException::~ParseException() throw() {}

const char* ParseException::what() const throw()
{
	return message.c_str();
}
