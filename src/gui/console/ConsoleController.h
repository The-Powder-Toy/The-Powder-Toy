#pragma once
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
	void EvaluateCommand(const String& command);
	void NextCommand();
	void PreviousCommand();
	void Exit();
	void CloseConsole();
	ConsoleView * GetView();
	virtual ~ConsoleController();
};
