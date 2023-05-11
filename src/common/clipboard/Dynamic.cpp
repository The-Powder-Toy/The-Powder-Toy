#include "Dynamic.h"
#include "client/GameSave.h"
#include "prefs/GlobalPrefs.h"
#include "PowderToySDL.h"
#include <SDL_syswm.h>
#include <iostream>

namespace Clipboard
{
#define CLIPBOARD_IMPLS_DECLARE
#include "ClipboardImpls.h"
#undef CLIPBOARD_IMPLS_DECLARE

	struct ClipboardImplEntry
	{
		SDL_SYSWM_TYPE subsystem;
		std::unique_ptr<ClipboardImpl> (*factory)();
	} clipboardImpls[] = {
#define CLIPBOARD_IMPLS_DEFINE
#include "ClipboardImpls.h"
#undef CLIPBOARD_IMPLS_DEFINE
		{ SDL_SYSWM_UNKNOWN, nullptr },
	};

	std::unique_ptr<GameSave> clipboardData;
	std::unique_ptr<ClipboardImpl> clipboard;

	void InvokeClipboardSetClipboardData()
	{
		if (clipboard)
		{
			if (clipboardData)
			{
				clipboard->SetClipboardData(); // this either works or it doesn't, we don't care
			}
			else
			{
				std::cerr << "cannot put save on clipboard: no data to transfer" << std::endl;
			}
		}
	}

	void SerializeClipboard(std::vector<char> &saveData)
	{
		std::tie(std::ignore, saveData) = clipboardData->Serialise();
	}

	void SetClipboardData(std::unique_ptr<GameSave> data)
	{
		clipboardData = std::move(data);
		InvokeClipboardSetClipboardData();
	}

	void InvokeClipboardGetClipboardData()
	{
		if (clipboard)
		{
			auto result = clipboard->GetClipboardData();
			if (std::holds_alternative<ClipboardImpl::GetClipboardDataUnchanged>(result))
			{
				std::cerr << "not getting save from clipboard, data unchanged" << std::endl;
				return;
			}
			if (std::holds_alternative<ClipboardImpl::GetClipboardDataUnknown>(result))
			{
				return;
			}
			clipboardData.reset();
			auto *data = std::get_if<ClipboardImpl::GetClipboardDataChanged>(&result);
			if (!data)
			{
				return;
			}
			try
			{
				clipboardData = std::make_unique<GameSave>(data->data);
			}
			catch (const ParseException &e)
			{
				std::cerr << "got bad save from clipboard: " << e.what() << std::endl;
				return;
			}
			std::cerr << "got save from clipboard" << std::endl;
		}
	}

	const GameSave *GetClipboardData()
	{
		InvokeClipboardGetClipboardData();
		return clipboardData.get();
	}

	void Init()
	{
	}

	int currentSubsystem;

	void RecreateWindow()
	{
		// old window is gone (or doesn't exist), associate clipboard data with the new one
		SDL_SysWMinfo info;
		SDL_VERSION(&info.version);
		SDL_GetWindowWMInfo(sdl_window, &info);
		clipboard.reset();
		currentSubsystem = info.subsystem;
		if (GlobalPrefs::Ref().Get<bool>("NativeClipboard.Enabled", true))
		{
			for (auto *impl = clipboardImpls; impl->factory; ++impl)
			{
				if (impl->subsystem == currentSubsystem)
				{
					clipboard = impl->factory();
					break;
				}
			}
		}
		InvokeClipboardSetClipboardData();
	}
}
