
#include <string>
#include "Console.h"

int Console::ParseType(std::string * type)
{
	char * txt = (char *)type->c_str();
	int i = -1;
	// alternative names for some elements
	if (*type == "C4") i = PT_PLEX;
	else if (*type == "C5") i = PT_C5;
	else if (*type == "NONE") i = PT_NONE;
	if (i>=0 && i<PT_NUM && sim->ptypes[i].enabled)
	{
		(*lastError) = "";
		return i;
	}
	for (i=1; i<PT_NUM; i++) {
		if (strcasecmp(txt, sim->ptypes[i].name)==0 && sim->ptypes[i].enabled)
		{
			(*lastError) = "";
			return i;
		}
	}
	(*lastError) = "Particle type not recognised";
	return -1;
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

int Console::ProcessCommand(std::string * command)
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
