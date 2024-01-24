#pragma once
#include "common/String.h"
#include "gui/game/GameControllerEvents.h"
#include "TPTSTypes.h"
#include <deque>

class GameModel;
class GameController;
class Tool;

class CommandInterface
{
protected:
	String lastError;
	GameModel * m;
	GameController * c;
	CommandInterface(GameController *newGameController, GameModel *newGameModel);

public:
	enum LogType { LogError, LogWarning, LogNotice };
	enum FormatType { FormatInt, FormatString, FormatChar, FormatFloat, FormatElement };
	int GetPropertyOffset(ByteString key, FormatType & format);
	void Log(LogType type, String message);
	//void AttachGameModel(GameModel * m);

	virtual void OnTick() { }
	virtual void Init() { }

	virtual bool HandleEvent(const GameControllerEvent &event) { return true; }

	virtual int Command(String command);
	virtual String FormatCommand(String command);
	void SetLastError(String err)
	{
		lastError = err;
	}
	String GetLastError();
	virtual ~CommandInterface();

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

	static CommandInterface *Create(GameController *newGameController, GameModel *newGameModel);
};

extern CommandInterface *commandInterface;
