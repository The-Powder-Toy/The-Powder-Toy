#pragma once
#include <set>
#include <map>
#include "common/String.h"

struct MissingElements
{
	std::map<ByteString, int> identifiers;
	std::set<int> ids;
};
