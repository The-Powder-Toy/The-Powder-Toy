#include "ToolClasses.h"
std::vector<SimTool*> GetTools()
{
	std::vector<SimTool*> tools;
	tools.push_back(new Tool_Heat());
	tools.push_back(new Tool_Cool());
	tools.push_back(new Tool_Air());
	tools.push_back(new Tool_Vac());
	tools.push_back(new Tool_PGrv());
	tools.push_back(new Tool_NGrv());
	return tools;
}
