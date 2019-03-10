#ifndef GAMEMODELEXCEPTION_H_
#define GAMEMODELEXCEPTION_H_

#include "common/String.h"
#include <exception>

struct GameModelException: public exception {
	String message;
public:
	GameModelException(String message_): message(message_) {}
	const char * what() const throw() override
	{
		return message.ToUtf8().c_str();
	}
	~GameModelException() throw() {}
};

#endif /* GAMEMODELEXCEPTION_H_ */
