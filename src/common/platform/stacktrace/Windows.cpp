#include "common/platform/Platform.h"
#include "common/Defer.h"
#include <windows.h>
#pragma pack(push, 8)
#include <dbghelp.h>
#pragma pack(pop)
#include <psapi.h>
#include <array>
#include <mutex>
#include <cstdint>

namespace Platform
{
struct SymbolInfo
{
	String name;
	uintptr_t displacement;
};
static std::optional<SymbolInfo> GetSymbolInfo(HANDLE process, uintptr_t offset)
{
	DWORD64 displacement;
	std::array<char, sizeof(SYMBOL_INFOW) + 1000> symbolData{};
	auto &symbol = *reinterpret_cast<SYMBOL_INFOW *>(symbolData.data());
	symbol.SizeOfStruct = sizeof(symbol);
	symbol.MaxNameLen = symbolData.size() - sizeof(symbol);
	if (SymFromAddrW(process, offset, &displacement, &symbol))
	{
		return SymbolInfo{ WinNarrow(&symbol.Name[0]).FromUtf8(), uintptr_t(displacement) };
	}
	return std::nullopt;
}

struct ModuleInfo
{
	String name;
	uintptr_t displacement;
};
static std::optional<ModuleInfo> GetModuleInfo(HANDLE process, uintptr_t offset)
{
	IMAGEHLP_MODULEW64 module{};
	module.SizeOfStruct = sizeof(module);
	if (SymGetModuleInfoW64(process, offset, &module))
	{
		auto displacement = offset - uintptr_t(module.BaseOfImage);
		return ModuleInfo{ WinNarrow(&module.LoadedImageName[0]).FromUtf8(), displacement };
	}
	return std::nullopt;
}

std::optional<std::vector<String>> StackTrace()
{
	static std::mutex mx;
	std::unique_lock lk(mx);
	auto process = GetCurrentProcess();
	auto thread = GetCurrentThread();

	Defer symCleanup([process]() {
		SymCleanup(process);
	});
	SymInitialize(process, NULL, TRUE);

	CONTEXT context{};
	context.ContextFlags = CONTEXT_FULL;
	RtlCaptureContext(&context);

	STACKFRAME64 frame{};
	DWORD machine;
#if defined(_M_IX86)
	machine                 = IMAGE_FILE_MACHINE_I386;
	frame.AddrPC.Offset     = context.Eip;
	frame.AddrPC.Mode       = AddrModeFlat;
	frame.AddrFrame.Offset  = context.Ebp;
	frame.AddrFrame.Mode    = AddrModeFlat;
	frame.AddrStack.Offset  = context.Esp;
	frame.AddrStack.Mode    = AddrModeFlat;
#elif defined(_M_X64)
	machine                 = IMAGE_FILE_MACHINE_AMD64;
	frame.AddrPC.Offset     = context.Rip;
	frame.AddrPC.Mode       = AddrModeFlat;
	frame.AddrFrame.Offset  = context.Rsp;
	frame.AddrFrame.Mode    = AddrModeFlat;
	frame.AddrStack.Offset  = context.Rsp;
	frame.AddrStack.Mode    = AddrModeFlat;
#elif defined(_M_IA64)
	machine                 = IMAGE_FILE_MACHINE_IA64;
	frame.AddrPC.Offset     = context.StIIP;
	frame.AddrPC.Mode       = AddrModeFlat;
	frame.AddrFrame.Offset  = context.IntSp;
	frame.AddrFrame.Mode    = AddrModeFlat;
	frame.AddrBStore.Offset = context.RsBSP;
	frame.AddrBStore.Mode   = AddrModeFlat;
	frame.AddrStack.Offset  = context.IntSp;
	frame.AddrStack.Mode    = AddrModeFlat;
#elif defined(_M_ARM64)
	machine                 = IMAGE_FILE_MACHINE_ARM64;
	frame.AddrPC.Offset     = context.Pc;
	frame.AddrPC.Mode       = AddrModeFlat;
	frame.AddrFrame.Offset  = context.Fp;
	frame.AddrFrame.Mode    = AddrModeFlat;
	frame.AddrStack.Offset  = context.Sp;
	frame.AddrStack.Mode    = AddrModeFlat;
#else
	return std::nullopt;
#endif

	std::vector<String> res;
	for (auto i = 0; i < 100; ++i)
	{
		if (!StackWalk64(machine, process, thread, &frame, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
		{
			break;
		}
		auto offset = uintptr_t(frame.AddrPC.Offset);
		StringBuilder addr;
		addr << Format::Hex();
		if (auto moduleInfo = GetModuleInfo(process, offset))
		{
			addr << moduleInfo->name << "(";
			if (auto symbolInfo = GetSymbolInfo(process, offset))
			{
				addr << symbolInfo->name << "+0x" << symbolInfo->displacement;
			}
			else
			{
				addr << "+0x" << moduleInfo->displacement;
			}
			addr << ") [0x" << offset << "]";
		}
		else
		{
			addr << "0x" << offset;
		}
		res.push_back(addr.Build());
	}
	return res;
}
}
