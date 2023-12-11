#pragma once
#include <array>

#include "SimulationData.h"
#include "Element.h"

#define ELEMENT_NUMBERS_ENUMERATE
#include "ElementNumbers.h"
#undef ELEMENT_NUMBERS_ENUMERATE

std::array<Element, PT_NUM> const &GetElements();
