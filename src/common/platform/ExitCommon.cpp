#include "Platform.h"
#include <cstdlib>
#include <list>

namespace Platform
{
std::list<ExitFunc> exitFuncs;

void Atexit(ExitFunc exitFunc)
{
	exitFuncs.push_front(exitFunc);
}

void Exit(int code)
{
	for (auto exitFunc : exitFuncs)
	{
		exitFunc();
	}
	exit(code);
}
}
