#pragma once
#include "common/String.h"
#include <optional>

namespace Platform
{
	std::optional<ByteString> CallActivityStringFunc(const char *funcName);
}
