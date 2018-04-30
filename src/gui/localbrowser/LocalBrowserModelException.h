#ifndef STAMPSMODELEXCEPTION_H_
#define STAMPSMODELEXCEPTION_H_

#include "common/String.h"
#include <exception>

class LocalBrowserModelException {
	String message;
public:
	LocalBrowserModelException(String message_): message(message_) {};
	const char * what() const throw() { return message.ToUtf8().c_str(); };
	~LocalBrowserModelException() throw() {};
};

#endif /* STAMPSMODELEXCEPTION_H_ */
