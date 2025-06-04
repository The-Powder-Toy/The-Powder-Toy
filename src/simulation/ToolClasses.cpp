#include "ToolClasses.h"
#include "ToolNumbers.h"

std::vector<SimTool> const &GetTools()
{
	struct DoOnce
	{
		std::vector<SimTool> tools;

		DoOnce()
		{
#define TOOL_NUMBERS_CALL(name, id) tools.push_back(SimTool()), tools.back().Tool_ ## name ();
TOOL_NUMBERS(TOOL_NUMBERS_CALL)
#undef TOOL_NUMBERS_CALL
		}
	};

	static DoOnce doOnce;
	return doOnce.tools;
}
