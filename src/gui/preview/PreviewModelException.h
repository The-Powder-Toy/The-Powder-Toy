#ifndef PREVIEWMODELEXCEPTION_H_
#define PREVIEWMODELEXCEPTION_H_

#include "common/String.h"
#include <exception>

struct PreviewModelException: public std::exception
{
	ByteString message;
public:
	PreviewModelException(String message_): message(message_.ToUtf8()) {}
	const char * what() const throw() override
	{
		return message.c_str();
	}
	~PreviewModelException() throw() {}
};

#endif /* PREVIEWMODELEXCEPTION_H_ */
