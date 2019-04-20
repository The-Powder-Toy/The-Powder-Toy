#ifndef GAMEMODELEXCEPTION_H_
#define GAMEMODELEXCEPTION_H_

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

#endif /* GAMEMODELEXCEPTION_H_ */
