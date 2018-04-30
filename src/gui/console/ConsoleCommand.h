#ifndef CONSOLECOMMAND_H_
#define CONSOLECOMMAND_H_

class ConsoleCommand
{
public:
	ConsoleCommand(String command, int returnStatus, String returnValue):
		Command(command), ReturnStatus(returnStatus), ReturnValue(returnValue)
	{

	}
	String Command;
	int ReturnStatus;
	String ReturnValue;

	operator ByteString() const
	{
		return Command.ToUtf8();
	}
};


#endif /* CONSOLECOMMAND_H_ */
