#include "VirtualMachine.h"
#include <cstdio>
#include <cstdlib>

namespace vm
{
	#define ARG(n) (Get<int4_t>(RP + ((2 + n) * sizeof(word))))

	#define TRAPDEF(f) int VirtualMachine::trap##f()

	TRAPDEF(Print)
	{

		char *text;

		//crumb("SYSCALL Print [%d]\n", ARG(0)); 
		text = (char*)(ram) + ARG(0);
		//crumb("PRINTING [%s]\n", text);
		printf("%s", text);
		return 0;
	}


	TRAPDEF(Error)
	{
		char *msg;

		msg = (char*)(ram) + ARG(0);
		printf("%s", msg);
		PC = romSize + 1;
		return 0;
	}
}