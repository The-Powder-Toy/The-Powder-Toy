#ifndef TAGSMODELEXCEPTION_H_
#define TAGSMODELEXCEPTION_H_

#include "common/String.h"
#include <exception>

class TagsModelException : public std::exception {
	ByteString message;
public:
	TagsModelException(String message): message(message.ToUtf8()) {};
	const char * what() const throw() override { return message.c_str(); };
	~TagsModelException() throw() {};
};

#endif /* TAGSMODELEXCEPTION_H_ */
