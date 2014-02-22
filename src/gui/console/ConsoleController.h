#ifndef CONSOLECONTROLLER_H_
#define CONSOLECONTROLLER_H_

#include <string>
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
	std::string FormatCommand(std::string command);
	void EvaluateCommand(std::string command);
	void NextCommand();
	void PreviousCommand();
	void Exit();
	void CloseConsole();
	ConsoleView * GetView();
	virtual ~ConsoleController();
};

#endif /* CONSOLECONTROLLER_H_ */
