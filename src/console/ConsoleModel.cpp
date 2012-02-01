/*
 * ConsoleModel.cpp
 *
 *  Created on: Feb 1, 2012
 *      Author: Simon
 */

#include "ConsoleModel.h"

ConsoleModel::ConsoleModel() {

}

void ConsoleModel::AddObserver(ConsoleView * observer)
{
	observers.push_back(observer);
}

int ConsoleModel::GetCurrentCommandIndex()
{
	return currentCommandIndex;
}

void ConsoleModel::SetCurrentCommandIndex(int index)
{
	currentCommandIndex = index;
	notifyCurrentCommandChanged();
}

ConsoleCommand ConsoleModel::GetCurrentCommand()
{
	if(currentCommandIndex < 0 || currentCommandIndex >= previousCommands.size())
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
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyPreviousCommandsChanged(this);
	}
}

void ConsoleModel::notifyCurrentCommandChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyCurrentCommandChanged(this);
	}
}

ConsoleModel::~ConsoleModel() {

}

