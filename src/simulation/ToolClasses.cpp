#include "ToolClasses.h"

std::vector<SimTool> const &GetTools()
{
	struct DoOnce
	{
		std::vector<SimTool> tools;

		DoOnce()
		{
#define TOOL_NUMBERS_CALL
#include "ToolNumbers.h"
#undef TOOL_NUMBERS_CALL
		}
	};

	static DoOnce doOnce;
	return doOnce.tools;
}
