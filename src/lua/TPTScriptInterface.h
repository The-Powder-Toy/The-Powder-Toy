#ifndef TPTSCRIPTINTERFACE_H_
#define TPTSCRIPTINTERFACE_H_
#include "Config.h"

#include "CommandInterface.h"
#include "TPTSTypes.h"
#include <deque>

class TPTScriptInterface: public CommandInterface {
protected:
	AnyType eval(std::deque<String> * words);
	int parseNumber(String str);
	AnyType tptS_set(std::deque<String> * words);
	AnyType tptS_create(std::deque<String> * words);
	AnyType tptS_delete(std::deque<String> * words);
	AnyType tptS_load(std::deque<String> * words);
	AnyType tptS_reset(std::deque<String> * words);
	AnyType tptS_bubble(std::deque<String> * words);
	AnyType tptS_quit(std::deque<String> * words);
	ValueType testType(String word);
public:
	TPTScriptInterface(GameController * c, GameModel * m);
	int Command(String command) override;
	String FormatCommand(String command) override;
	virtual ~TPTScriptInterface();
};

#endif /* TPTSCRIPTINTERFACE_H_ */
