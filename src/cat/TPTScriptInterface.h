#ifndef TPTSCRIPTINTERFACE_H_
#define TPTSCRIPTINTERFACE_H_

#include "CommandInterface.h"
#include "TPTSTypes.h"

class TPTScriptInterface: public CommandInterface {
protected:
	AnyType eval(std::deque<std::string> * words);
	int parseNumber(char * stringData);
	AnyType tptS_set(std::deque<std::string> * words);
	AnyType tptS_create(std::deque<std::string> * words);
	AnyType tptS_delete(std::deque<std::string> * words);
	AnyType tptS_load(std::deque<std::string> * words);
	AnyType tptS_reset(std::deque<std::string> * words);
	AnyType tptS_bubble(std::deque<std::string> * words);
	AnyType tptS_quit(std::deque<std::string> * words);
	ValueType testType(std::string word);
public:
	TPTScriptInterface(GameController * c, GameModel * m);
	virtual void Tick() {}
	virtual int Command(std::string command);
	virtual std::string FormatCommand(std::string command);
	virtual ~TPTScriptInterface();
};

#endif /* TPTSCRIPTINTERFACE_H_ */
