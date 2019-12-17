#ifndef CONSOLECONTROLLER_H_
#define CONSOLECONTROLLER_H_
#include "Config.h"

#include "common/String.h"

#include <functional>

class CommandInterface;
class ConsoleModel;
class ConsoleView;
class ConsoleController
{
	ConsoleView * consoleView;
	ConsoleModel * consoleModel;
	CommandInterface * commandInterface;
	std::function<void ()> onDone;
public:
	bool HasDone;
	ConsoleController(std::function<void ()> onDone, CommandInterface * commandInterface);
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
