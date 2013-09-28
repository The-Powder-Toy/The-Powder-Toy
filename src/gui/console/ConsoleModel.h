#ifndef CONSOLEMODEL_H_
#define CONSOLEMODEL_H_

#include <vector>
#include <deque>
#include "ConsoleView.h"
#include "cat/CommandInterface.h"

class ConsoleView;
class ConsoleModel {
	int currentCommandIndex;
	std::vector<ConsoleView*> observers;
	std::deque<std::string> previousCommands;
	std::string command;
	std::string promptHistory;
	std::string history;
	void notifyPreviousCommandsChanged();
	void notifyCurrentCommandChanged();
	void notifyHistoryChanged();
public:
	void NextCommand();
	void PreviousCommand();

	ConsoleModel();
	void AddObserver(ConsoleView * observer);
	void ProcessResult(std::string command, std::string highlighted, CommandInterface::EvalResult * result);
	virtual ~ConsoleModel();
};

#endif /* CONSOLEMODEL_H_ */
