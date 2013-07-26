#ifndef TAGSMODELEXCEPTION_H_
#define TAGSMODELEXCEPTION_H_

#include <string>
#include <exception>

class TagsModelException {
	std::string message;
public:
	TagsModelException(std::string message_): message(message_) {};
	const char * what() const throw() { return message.c_str(); };
	~TagsModelException() throw() {};
};

#endif /* TAGSMODELEXCEPTION_H_ */
