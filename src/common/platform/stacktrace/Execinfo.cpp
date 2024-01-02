#include "common/platform/Platform.h"
#include "common/Defer.h"
#include <execinfo.h>
#include <cstdint>
#include <array>
#include <cxxabi.h>
#include <iostream>

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
			auto line = ByteString(strs[i]);
			if (auto beginSymbolName = line.SplitBy('('))
			{
				auto afterBeginSymbolName = beginSymbolName.After();
				if (auto endSymbolName = afterBeginSymbolName.SplitBy('+'))
				{
					auto beforeSymbolName = beginSymbolName.Before();
					auto symbolName = endSymbolName.Before();
					auto afterSymbolName = endSymbolName.After();
					int status;
					char *demangled = abi::__cxa_demangle(symbolName.c_str(), NULL, NULL, &status);
					Defer freeDemangled([demangled]() {
						free(demangled);
					});
					if (!status)
					{
						line = ByteString::Build(beforeSymbolName, "(", demangled, "+", afterSymbolName);
					}
				}
			}
			res.push_back(line.FromUtf8());
		}
		else
		{
			res.push_back(String::Build("0x", Format::Hex(), uintptr_t(buf[i])));
		}
	}
	return res;
}
}
