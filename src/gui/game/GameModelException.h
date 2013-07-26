#ifndef GAMEMODELEXCEPTION_H_
#define GAMEMODELEXCEPTION_H_

#include <string>
#include <exception>
using namespace std;

struct GameModelException: public exception {
	string message;
public:
	GameModelException(string message_): message(message_) {}
	const char * what() const throw()
	{
		return message.c_str();
	}
	~GameModelException() throw() {};
};

#endif /* GAMEMODELEXCEPTION_H_ */
