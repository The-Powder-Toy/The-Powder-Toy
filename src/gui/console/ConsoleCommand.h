#pragma once
#include "common/String.h"

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

	operator String() const
	{
		return Command;
	}
};

