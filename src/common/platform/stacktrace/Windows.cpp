#include "common/platform/Platform.h"
#include "common/Defer.h"
#include <windows.h>
#pragma pack(push, 8)
#include <dbghelp.h>
#pragma pack(pop)
#include <psapi.h>
#include <array>
#include <mutex>

namespace Platform
{
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

	CONTEXT context;
	memset(&context, 0, sizeof(context));
	context.ContextFlags = CONTEXT_FULL;
	RtlCaptureContext(&context);

	STACKFRAME64 frame;
	memset(&frame, 0, sizeof(frame));
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
		if (!StackWalk64( machine, process, thread, &frame, &context, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
		{
			break;
		}
		StringBuilder addr;
		addr << "0x" << Format::Hex() << uintptr_t(frame.AddrPC.Offset);
		std::array<wchar_t, 1000> moduleBaseName;
		HMODULE module;
		MODULEINFO moduleInfo;
		if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)frame.AddrPC.Offset, &module) &&
		    GetModuleBaseNameW(process, module, &moduleBaseName[0], moduleBaseName.size()) &&
		    GetModuleInformation(process, module, &moduleInfo, sizeof(moduleInfo)))
		{
			auto offset = uintptr_t(frame.AddrPC.Offset) - uintptr_t(moduleInfo.lpBaseOfDll);
			if (offset < moduleInfo.SizeOfImage)
			{
				addr << " (" << WinNarrow(&moduleBaseName[0]).FromUtf8() << "+0x" << Format::Hex() << offset << ")";
			}
		}
		res.push_back(addr.Build());
	}
	return res;
}
}
