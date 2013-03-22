#ifndef CONSOLECOMMAND_H_
#define CONSOLECOMMAND_H_

class ConsoleCommand
{
public:
	ConsoleCommand(std::string command, int returnStatus, std::string returnValue):
		Command(command), ReturnStatus(returnStatus), ReturnValue(returnValue)
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
