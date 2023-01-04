#pragma once
#include "common/String.h"
#include <exception>

struct GameModelException: public std::exception
{
	ByteString message;
public:
	GameModelException(String message_): message(message_.ToUtf8()) {}
	const char * what() const throw() override
	{
		return message.c_str();
	}
	~GameModelException() throw() {}
};
