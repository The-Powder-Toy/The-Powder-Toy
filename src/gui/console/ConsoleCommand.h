#include <utility>

#ifndef CONSOLECOMMAND_H_
#define CONSOLECOMMAND_H_

class ConsoleCommand
{
public:
	ConsoleCommand(std::string command, int returnStatus, std::string returnValue):
		Command(std::move(command)), ReturnStatus(returnStatus), ReturnValue(std::move(returnValue))
	{

	}
	std::string Command;
	int ReturnStatus;
	std::string ReturnValue;

	operator std::string() const
	{
		return Command;
	}
};


#endif /* CONSOLECOMMAND_H_ */
