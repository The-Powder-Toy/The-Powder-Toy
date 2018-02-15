#ifndef STAMPSMODELEXCEPTION_H_
#define STAMPSMODELEXCEPTION_H_

#include <string>
#include <exception>
#include <utility>
#include <utility>
using namespace std;

class LocalBrowserModelException {
	string message;
public:
	LocalBrowserModelException(string message_): message(std::move(message_)) {};
	const char * what() const throw() { return message.c_str(); };
	~LocalBrowserModelException() throw() = default;;
};

#endif /* STAMPSMODELEXCEPTION_H_ */
