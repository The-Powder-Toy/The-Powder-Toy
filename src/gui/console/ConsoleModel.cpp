#include "client/Client.h"
#include "ConsoleModel.h"

ConsoleModel::ConsoleModel() {
	std::vector<std::string> previousHistory = Client::Ref().GetPrefStringArray("Console.History");
	for(std::vector<std::string>::reverse_iterator iter = previousHistory.rbegin(), end = previousHistory.rend(); iter != end; ++iter)
	{
		if(previousCommands.size()<25)
		{
			previousCommands.push_front(ConsoleCommand(*iter, 0, ""));
			currentCommandIndex = previousCommands.size();
		}
	}
}

void ConsoleModel::AddObserver(ConsoleView * observer)
{
	observers.push_back(observer);
	observer->NotifyPreviousCommandsChanged(this);
}

size_t ConsoleModel::GetCurrentCommandIndex()
{
	return currentCommandIndex;
}

void ConsoleModel::SetCurrentCommandIndex(size_t index)
{
	currentCommandIndex = index;
	notifyCurrentCommandChanged();
}

ConsoleCommand ConsoleModel::GetCurrentCommand()
{
	if (currentCommandIndex >= previousCommands.size())
	{
		return ConsoleCommand("", 0, "");
	}
	return previousCommands[currentCommandIndex];
}

void ConsoleModel::AddLastCommand(ConsoleCommand command)
{
	previousCommands.push_back(command);
	if(previousCommands.size()>25)
		previousCommands.pop_front();
	currentCommandIndex = previousCommands.size();
	notifyPreviousCommandsChanged();
}

std::deque<ConsoleCommand> ConsoleModel::GetPreviousCommands()
{
	return previousCommands;
}

void ConsoleModel::notifyPreviousCommandsChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyPreviousCommandsChanged(this);
	}
}

void ConsoleModel::notifyCurrentCommandChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyCurrentCommandChanged(this);
	}
}

ConsoleModel::~ConsoleModel() {
	Client::Ref().SetPref("Console.History", std::vector<Json::Value>(previousCommands.begin(), previousCommands.end()));
}

