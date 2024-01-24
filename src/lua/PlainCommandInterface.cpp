#include "CommandInterface.h"

CommandInterfacePtr CommandInterface::Create(GameController *newGameController, GameModel *newGameModel)
{
	return CommandInterfacePtr(new CommandInterface(newGameController, newGameModel));
}

void CommandInterfaceDeleter::operator ()(CommandInterface *ptr) const
{
	delete ptr;
}

void CommandInterface::OnTick()
{
}

void CommandInterface::Init()
{
}

bool CommandInterface::HandleEvent(const GameControllerEvent &event)
{
	return true;
}

int CommandInterface::Command(String command)
{
	return PlainCommand(command);
}

String CommandInterface::FormatCommand(String command)
{
	return PlainFormatCommand(command);
}
