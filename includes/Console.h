#ifndef CONSOLE_H
#define CONSOLE_H

#include <string>
#include <vector>

#include "interface/Sandbox.h"
#include "Simulation.h"

class ConsoleCommand
{
private:
	std::string * command;
	int returnStatus;
	std::string * error;
public:
	void SetCommand(std::string * command);
	void SetError(std::string * error);
	std::string * GetCommand();
	std::string * GetError();
	ConsoleCommand();
	ConsoleCommand(std::string * command, int returnStatus, std::string * error = new std::string(""));
};

class Console
{
private:
	std::vector<ConsoleCommand> * previousCommands;
	std::string * lastError;
	ui::Sandbox * sandbox;
	Simulation * sim;
public:
	virtual void Tick(float * dt);
	int ParseType(std::string * type);
	virtual void ConsoleShown();
	virtual void ConsoleHidden();
	virtual int ProcessCommand(std::string * command);
	virtual std::string * GetLastError();
	virtual std::vector<ConsoleCommand> * GetPreviousCommands();
	Console(ui::Sandbox * sandbox);
	virtual ~Console();
};

#endif // CONSOLE_H
