#ifndef CONSOLECONTROLLER_H_
#define CONSOLECONTROLLER_H_

#include "common/String.h"
#include "Controller.h"
#include "ConsoleView.h"
#include "ConsoleModel.h"
#include "ConsoleCommand.h"
#include "lua/CommandInterface.h"

class ConsoleModel;
class ConsoleView;
class ConsoleController {
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
