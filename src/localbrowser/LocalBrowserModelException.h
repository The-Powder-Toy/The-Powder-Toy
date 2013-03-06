#ifndef STAMPSMODELEXCEPTION_H_
#define STAMPSMODELEXCEPTION_H_

#include <string>
#include <exception>
using namespace std;

class LocalBrowserModelException {
	string message;
public:
	LocalBrowserModelException(string message_): message(message_) {};
	const char * what() const throw() { return message.c_str(); };
	~LocalBrowserModelException() throw() {};
};

#endif /* STAMPSMODELEXCEPTION_H_ */
