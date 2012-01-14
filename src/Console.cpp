
#include <string>
#include <math.h>
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

Console::Console(ui::Sandbox * sandbox)
{
	this->sandbox = sandbox;
	sim = sandbox->GetSimulation();
}

Console::~Console()
{

}
