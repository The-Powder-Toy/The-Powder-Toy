#pragma once
#include "SimTool.h"
#include "ToolNumbers.h"
#include <vector>
#include <memory>

#define TOOL_NUMBERS_ENUMERATE(name, id) constexpr int TOOL_ ## name = id;
TOOL_NUMBERS(TOOL_NUMBERS_ENUMERATE)
#undef TOOL_NUMBERS_ENUMERATE

std::vector<SimTool> const &GetTools();
