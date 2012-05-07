
#include <string>
#include <cmath>
#include "Console.h"

int Console::ParsePartref(char * txt)
{

}

int Console::ParseCoords(char * coords, int *x, int *y)
{

}

int Console::ParseType(char * txt)
{

}

void Console::Tick(float * dt)
{

}

void Console::ConsoleShown()
{

}

void Console::ConsoleHidden()
{

}

int Console::ProcessCommand(char * console)
{

}

std::string * Console::GetLastError()
{
	return lastError;
}

std::vector<ConsoleCommand> * Console::GetPreviousCommands()
{

}

Console::Console()
{
	//sim = sandbox->GetSimulation();
}

Console::~Console()
{

}
