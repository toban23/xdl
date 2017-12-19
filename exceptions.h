#ifndef _GPExceptions
#define _GPExceptions

#include <exception>
#include <string>

class GPIOException : public std::exception
{
	std::string msg_;

public:
	explicit GPIOException(const char* message):
		msg_(message)
	{}

	explicit GPIOException(const std::string& message) :
		msg_(message)
	{}

	virtual ~GPIOException() throw() {}

	virtual const char* what() const throw()
	{
		return msg_.c_str();
	}
};

#endif //_GPExceptions