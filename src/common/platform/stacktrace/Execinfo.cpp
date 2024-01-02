#include "common/platform/Platform.h"
#include "common/Defer.h"
#include <execinfo.h>
#include <cstdint>
#include <array>

namespace Platform
{
std::optional<std::vector<String>> StackTrace()
{
	std::array<void *, 100> buf;
	auto used = backtrace(&buf[0], buf.size());
	auto *strs = backtrace_symbols(&buf[0], used);
	Defer freeStrs([strs]() {
		free(strs);
	});
	std::vector<String> res;
	for (auto i = 0; i < used; ++i)
	{
		if (strs)
		{
			res.push_back(ByteString(strs[i]).FromUtf8());
		}
		else
		{
			res.push_back(String::Build("0x", Format::Hex(), uintptr_t(buf[i])));
		}
	}
	return res;
}
}
