#ifndef CONSOLEMODEL_H_
#define CONSOLEMODEL_H_

#include <vector>
#include <deque>
#include "ConsoleView.h"
#include "ConsoleCommand.h"

class ConsoleView;
class ConsoleModel {
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
	virtual ~ConsoleModel();
};

#endif /* CONSOLEMODEL_H_ */
