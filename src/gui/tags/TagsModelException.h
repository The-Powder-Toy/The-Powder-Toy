#ifndef TAGSMODELEXCEPTION_H_
#define TAGSMODELEXCEPTION_H_

#include <string>
#include <exception>
#include <utility>

class TagsModelException {
	std::string message;
public:
	TagsModelException(std::string message_): message(std::move(message_)) {};
	const char * what() const throw() { return message.c_str(); };
	~TagsModelException() throw() = default;;
};

#endif /* TAGSMODELEXCEPTION_H_ */
