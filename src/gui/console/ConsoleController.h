#ifndef CONSOLECONTROLLER_H_
#define CONSOLECONTROLLER_H_

#include "common/String.h"

class CommandInterface;
class ConsoleModel;
class ConsoleView;
class ControllerCallback;
class ConsoleController
{
	ControllerCallback * callback;
	ConsoleView * consoleView;
	ConsoleModel * consoleModel;
	CommandInterface * commandInterface;
public:
	bool HasDone;
	ConsoleController(ControllerCallback * callback, CommandInterface * commandInterface);
	String FormatCommand(String command);
	void EvaluateCommand(String command);
	void NextCommand();
	void PreviousCommand();
	void Exit();
	void CloseConsole();
	ConsoleView * GetView();
	virtual ~ConsoleController();
};

#endif /* CONSOLECONTROLLER_H_ */
