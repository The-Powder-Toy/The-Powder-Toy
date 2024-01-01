#include "common/platform/Platform.h"

namespace Platform
{
std::optional<std::vector<String>> StackTrace(StackTraceType)
{
	return std::nullopt;
}
}
