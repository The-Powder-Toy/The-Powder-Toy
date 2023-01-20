#pragma once
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
	CommandInterface(GameController * c, GameModel * m);

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

	static CommandInterface *Create(GameController * c, GameModel * m);
};

extern CommandInterface *commandInterface;
