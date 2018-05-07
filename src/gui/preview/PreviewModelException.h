#ifndef PREVIEWMODELEXCEPTION_H_
#define PREVIEWMODELEXCEPTION_H_

#include "common/String.h"
#include <exception>
using namespace std;

struct PreviewModelException: public exception {
	String message;
public:
	PreviewModelException(String message_): message(message_) {}
	const char * what() const throw()
	{
		return message.ToUtf8().c_str();
	}
	~PreviewModelException() throw() {};
};

#endif /* PREVIEWMODELEXCEPTION_H_ */
