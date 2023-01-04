#pragma once
#include "common/String.h"
#include <exception>

class LocalBrowserModelException : std::exception
{
	ByteString message;
public:
	LocalBrowserModelException(String message_): message(message_.ToUtf8()) {};
	const char * what() const throw() { return message.c_str(); };
	~LocalBrowserModelException() throw() {};
};
