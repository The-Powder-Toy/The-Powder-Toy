#ifndef CONSOLE_H
#define CONSOLE_H

#include <string>
#include <vector>

#include "interface/Sandbox.h"
#include "simulation/Simulation.h"

class ConsoleCommand
{
private:
	std::string * command;
	int returnStatus;
	std::string * returnString;
public:
	void SetCommand(std::string * command);
	void SetError(std::string * error);
	std::string * GetCommand();
	std::string * GetError();
	ConsoleCommand();
	ConsoleCommand(std::string * command, int returnStatus, std::string * returnString = new std::string(""));
};

class Console
{
private:
	bool sound_enable;
	bool file_script;
	std::vector<ConsoleCommand> * previousCommands;
	std::string * lastError;
	ui::Sandbox * sandbox;
	Simulation * sim;
public:
	virtual void Tick(float * dt);
	int ParseType(char * txt);
	int ParsePartref(char * txt);
	int ParseCoords(char * coords, int *x, int *y);
	virtual void ConsoleShown();
	virtual void ConsoleHidden();
	virtual int ProcessCommand(char * console);
	virtual std::string * GetLastError();
	virtual std::vector<ConsoleCommand> * GetPreviousCommands();
	Console(ui::Sandbox * sandbox);
	virtual ~Console();
};

#endif // CONSOLE_H
