#ifndef CONSOLECOMMAND_H_
#define CONSOLECOMMAND_H_

#include "Format.h"

class ConsoleCommand
{
public:
	ConsoleCommand(std::string command, int returnStatus, std::string returnValue):
		Command(command), ReturnStatus(returnStatus), ReturnValue(format::StringToWString(returnValue))
	{

	}

	ConsoleCommand(std::string command, int returnStatus, std::wstring returnValue):
		Command(command), ReturnStatus(returnStatus), ReturnValue(returnValue)
	{

	}

	std::string Command;
	int ReturnStatus;
	std::wstring ReturnValue;

	operator std::string() const
	{
		return Command;
	}
};


#endif /* CONSOLECOMMAND_H_ */
