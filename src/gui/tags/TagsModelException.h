#ifndef TAGSMODELEXCEPTION_H_
#define TAGSMODELEXCEPTION_H_

#include "common/String.h"
#include <exception>

class TagsModelException {
	String message;
public:
	TagsModelException(String message_): message(message_) {};
	const char * what() const throw() { return message.ToUtf8().c_str(); };
	~TagsModelException() throw() {};
};

#endif /* TAGSMODELEXCEPTION_H_ */
