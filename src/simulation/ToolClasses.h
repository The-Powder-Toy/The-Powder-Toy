#ifndef TOOLCLASSES_H
#define TOOLCLASSES_H

#include <vector>
#include <memory>

#include "SimTool.h"

#define TOOL_NUMBERS_ENUMERATE
#include "ToolNumbers.h"
#undef TOOL_NUMBERS_ENUMERATE

std::vector<SimTool> const &GetTools();

#endif
