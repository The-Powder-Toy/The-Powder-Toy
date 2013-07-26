#ifndef PREVIEWMODELEXCEPTION_H_
#define PREVIEWMODELEXCEPTION_H_

#include <string>
#include <exception>
using namespace std;

struct PreviewModelException: public exception {
	string message;
public:
	PreviewModelException(string message_): message(message_) {}
	const char * what() const throw()
	{
		return message.c_str();
	}
	~PreviewModelException() throw() {};
};

#endif /* PREVIEWMODELEXCEPTION_H_ */
