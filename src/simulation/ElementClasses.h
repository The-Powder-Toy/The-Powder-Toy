#pragma once
#include <vector>

#include "SimulationData.h"
#include "Element.h"

#define ELEMENT_NUMBERS_ENUMERATE
#include "ElementNumbers.h"
#undef ELEMENT_NUMBERS_ENUMERATE

std::vector<Element> const &GetElements();
