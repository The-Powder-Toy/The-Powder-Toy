#ifndef COMMANDINTERFACE_H_
#define COMMANDINTERFACE_H_

#include <string>
#include "interface/Engine.h"
//#include "game/GameModel.h"

class GameModel;
class GameController;
class Tool;
class CommandInterface {
protected:
	std::string lastError;
	GameModel * m;
	GameController * c;
public:
	enum LogType { LogError, LogWarning, LogNotice };
	enum FormatType { FormatInt, FormatString, FormatChar, FormatFloat };
	CommandInterface(GameController * c, GameModel * m);
	int GetPropertyOffset(std::string key_, FormatType & format);
	int GetParticleType(std::string type);
	void Log(LogType type, std::string message);
	//void AttachGameModel(GameModel * m);
	virtual bool OnBrushChanged(int brushType, int rx, int ry) {return true;}
	virtual bool OnActiveToolChanged(int toolSelection, Tool * tool) {return true;}
	virtual bool OnMouseMove(int x, int y, int dx, int dy) {return true;}
	virtual bool OnMouseDown(int x, int y, unsigned button) {return true;}
	virtual bool OnMouseUp(int x, int y, unsigned button) {return true;}
	virtual bool OnMouseWheel(int x, int y, int d) {return true;}
	virtual bool OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt) {return true;}
	virtual bool OnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt) {return true;}
	virtual void OnTick() {}
	virtual int Command(std::string command);
	virtual std::string FormatCommand(std::string command);
	std::string GetLastError();
	virtual ~CommandInterface();
};

#endif /* COMMANDINTERFACE_H_ */
