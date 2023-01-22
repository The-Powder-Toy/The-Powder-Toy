#include "ToolClasses.h"

std::vector<SimTool> const &GetTools()
{
	static std::vector<SimTool> tools;
	return tools;
}
