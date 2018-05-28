#ifndef COMMANDINTERFACE_H_
#define COMMANDINTERFACE_H_

#include "common/String.h"
#include "gui/interface/Engine.h"
//#include "game/GameModel.h"

class GameModel;
class GameController;
class Tool;
class CommandInterface {
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
	virtual bool OnActiveToolChanged(int toolSelection, Tool * tool) {return true;}
	virtual bool OnMouseMove(int x, int y, int dx, int dy) {return true;}
	virtual bool OnMouseDown(int x, int y, unsigned button) {return true;}
	virtual bool OnMouseUp(int x, int y, unsigned button, char type) {return true;}
	virtual bool OnMouseWheel(int x, int y, int d) {return true;}
	virtual bool OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) {return true;}
	virtual bool OnKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) {return true;}
	virtual bool OnMouseTick() { return true; }
	virtual void OnTick() { }
	virtual int Command(String command);
	virtual String FormatCommand(String command);
	String GetLastError();
	virtual ~CommandInterface();
};

#endif /* COMMANDINTERFACE_H_ */
