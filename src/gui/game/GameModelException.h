#ifndef GAMEMODELEXCEPTION_H_
#define GAMEMODELEXCEPTION_H_

#include <string>
#include <exception>
#include <utility>
#include <utility>
using namespace std;

struct GameModelException: public exception {
	string message;
public:
	GameModelException(string message_): message(std::move(message_)) {}
	const char * what() const throw() override
	{
		return message.c_str();
	}
	~GameModelException() throw() override = default;;
};

#endif /* GAMEMODELEXCEPTION_H_ */
