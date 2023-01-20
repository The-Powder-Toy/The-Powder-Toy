#pragma once
#include "ConsoleCommand.h"
#include <vector>
#include <deque>

class ConsoleView;
class ConsoleModel
{
	size_t currentCommandIndex;
	std::vector<ConsoleView*> observers;
	std::deque<ConsoleCommand> previousCommands;
	void notifyPreviousCommandsChanged();
	void notifyCurrentCommandChanged();
public:
	size_t GetCurrentCommandIndex();
	void SetCurrentCommandIndex(size_t index);
	ConsoleCommand GetCurrentCommand();

	std::deque<ConsoleCommand> GetPreviousCommands();
	ConsoleModel();
	void AddObserver(ConsoleView * observer);
	void AddLastCommand(ConsoleCommand command);
};
