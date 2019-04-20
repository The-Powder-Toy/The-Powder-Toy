#ifndef STAMPSMODELEXCEPTION_H_
#define STAMPSMODELEXCEPTION_H_

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

#endif /* STAMPSMODELEXCEPTION_H_ */
