#pragma once
#include "Config.h"

#include "common/String.h"
#include "gui/game/GameControllerEvents.h"

class GameModel;
class GameController;
class Tool;

class CommandInterface
{
protected:
	String lastError;
	GameModel * m;
	GameController * c;
public:
	enum LogType { LogError, LogWarning, LogNotice };
	enum FormatType { FormatInt, FormatString, FormatChar, FormatFloat, FormatElement };
	CommandInterface(GameController * c, GameModel * m);
	int GetPropertyOffset(ByteString key, FormatType & format);
	void Log(LogType type, String message);
	//void AttachGameModel(GameModel * m);

	virtual void OnTick() { }

	virtual bool HandleEvent(const GameControllerEvent &event) { return true; }

	virtual int Command(String command);
	virtual String FormatCommand(String command);
	void SetLastError(String err)
	{
		lastError = err;
	}
	String GetLastError();
	virtual ~CommandInterface();
};
