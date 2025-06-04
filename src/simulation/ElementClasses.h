#pragma once
#include "SimulationData.h"
#include "Element.h"
#include "ElementNumbers.h"
#include <array>

#define ELEMENT_NUMBERS_ENUMERATE(name, id) constexpr int PT_ ## name = id;
ELEMENT_NUMBERS(ELEMENT_NUMBERS_ENUMERATE);
#undef ELEMENT_NUMBERS_ENUMERATE

std::array<Element, PT_NUM> const &GetElements();
