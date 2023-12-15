#include "Dynamic.h"
#include "Clipboard.h"
#include "client/GameSave.h"
#include "prefs/GlobalPrefs.h"
#include "PowderToySDL.h"
#include <iostream>
#include <stdio.h>
#include <optional>
#include <algorithm>
#include <signal.h>
#include <map>
#include <SDL_syswm.h>

namespace Clipboard
{
	struct Preset
	{
		ByteString inCommand;
		ByteString formatsCommand;
		ByteString outCommand;
		std::optional<int> defaultForSubsystem;
	};
	std::map<ByteString, Preset> builtInPresets = {
		{ "xclip", {
			"xclip -selection clipboard -target %s",
			"xclip -out -selection clipboard -target TARGETS",
			"xclip -out -selection clipboard -target %s",
			SDL_SYSWM_X11,
		} },
		{ "wl-clipboard", {
			"wl-copy --type %s",
			"wl-paste --list-types",
			"wl-paste --type %s",
			SDL_SYSWM_WAYLAND,
		} },
	};

	static ByteString SubstFormat(ByteString str)
	{
		if (auto split = str.SplitBy("%s"))
		{
			str = split.Before() + clipboardFormatName + split.After();
		}
		return str;
	}

	static std::optional<Preset> GetPreset()
	{
		std::optional<ByteString> name = GlobalPrefs::Ref().Get("NativeClipboard.External.Type", ByteString("auto"));
		if (name == "custom")
		{
			auto getCommand = [](ByteString key) -> std::optional<ByteString> {
				auto fullKey = "NativeClipboard.External." + key;
				auto value = GlobalPrefs::Ref().Get<ByteString>(fullKey);
				if (!value)
				{
					std::cerr << "custom external clipboard command preset: missing " << fullKey << std::endl;
					return std::nullopt;
				}
				return *value;
			};
			auto inCommand      = getCommand("In");
			auto formatsCommand = getCommand("Formats");
			auto outCommand     = getCommand("Out");
			if (!inCommand || !formatsCommand || !outCommand)
			{
				return std::nullopt;
			}
			return Preset{
				SubstFormat(*inCommand),
				SubstFormat(*formatsCommand),
				SubstFormat(*outCommand),
			};
		}
		if (name == "auto")
		{
			name.reset();
			for (auto &[ presetName, preset ] : builtInPresets)
			{
				if (preset.defaultForSubsystem && *preset.defaultForSubsystem == currentSubsystem)
				{
					name = presetName;
				}
			}
			if (!name)
			{
				std::cerr << "no built-in external clipboard command preset for SDL window subsystem " << currentSubsystem << std::endl;
				return std::nullopt;
			}
		}
		auto it = builtInPresets.find(*name);
		if (it == builtInPresets.end())
		{
			std::cerr << "no built-in external clipboard command preset with name " << *name << std::endl;
			return std::nullopt;
		}
		return Preset{
			SubstFormat(it->second.inCommand),
			SubstFormat(it->second.formatsCommand),
			SubstFormat(it->second.outCommand),
		};
	}

	class ExternalClipboardImpl : public ClipboardImpl
	{
		bool initialized = false;

	public:
		ExternalClipboardImpl()
		{
			if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) // avoids problems with popen
			{
				std::cerr << "failed to initialize clipboard driver: signal: " << strerror(errno) << std::endl;
				return;
			}
			initialized = true;
		}

		void SetClipboardData() final override
		{
			if (!initialized)
			{
				std::cerr << "failed to set clipboard data: clipboard driver not initialized" << std::endl;
				return;
			}
			auto preset = GetPreset();
			if (!preset)
			{
				return;
			}
			auto handle = popen(preset->inCommand.c_str(), "we");
			if (!handle)
			{
				std::cerr << "failed to set clipboard data: popen: " << strerror(errno) << std::endl;
				return;
			}
			auto bail = false;
			std::vector<char> saveData;
			SerializeClipboard(saveData);
			if (fwrite(&saveData[0], 1, saveData.size(), handle) != saveData.size())
			{
				std::cerr << "failed to set clipboard data: fwrite: " << strerror(errno) << std::endl;
				bail = true;
			}
			auto status = pclose(handle);
			if (bail)
			{
				return;
			}
			if (status == -1)
			{
				std::cerr << "failed to set clipboard data: pclose: " << strerror(errno) << std::endl;
				return;
			}
			if (status)
			{
				std::cerr << "failed to set clipboard data: " << preset->inCommand << ": wait4 status code " << status << std::endl;
				return;
			}
		}

		GetClipboardDataResult GetClipboardData() final override
		{
			if (!initialized)
			{
				std::cerr << "cannot get save from clipboard: clipboard driver not initialized" << std::endl;
				return GetClipboardDataUnknown{};
			}
			auto getTarget = [](ByteString command) -> std::optional<std::vector<char>> {
				if (!command.size())
				{
					return std::nullopt;
				}
				auto handle = popen(command.c_str(), "re");
				if (!handle)
				{
					std::cerr << "cannot get save from clipboard: popen: " << strerror(errno) << std::endl;
					return std::nullopt;
				}
				constexpr auto blockSize = 0x10000;
				std::vector<char> data;
				auto bail = false;
				while (true)
				{
					auto pos = data.size();
					data.resize(pos + blockSize);
					auto got = fread(&data[pos], 1, blockSize, handle);
					if (got != blockSize)
					{
						if (ferror(handle))
						{
							std::cerr << "cannot get save from clipboard: fread: " << strerror(errno) << std::endl;
							bail = true;
							break;
						}
						if (feof(handle))
						{
							data.resize(data.size() - blockSize + got);
							break;
						}
					}
				}
				auto status = pclose(handle);
				if (bail)
				{
					return std::nullopt;
				}
				if (status == -1)
				{
					std::cerr << "cannot get save from clipboard: pclose: " << strerror(errno) << std::endl;
					return std::nullopt;
				}
				if (status)
				{
					std::cerr << "cannot get save from clipboard: " << command << ": wait4 status code " << status << std::endl;
					return std::nullopt;
				}
				return data;
			};
			auto preset = GetPreset();
			if (!preset)
			{
				return GetClipboardDataUnknown{};
			}
			auto formatsOpt = getTarget(preset->formatsCommand);
			if (!formatsOpt)
			{
				return GetClipboardDataUnknown{};
			}
			auto formats = ByteString(formatsOpt->begin(), formatsOpt->end()).PartitionBy('\n');
			if (std::find(formats.begin(), formats.end(), clipboardFormatName) == formats.end())
			{
				std::cerr << "not getting save from clipboard: no data" << std::endl;
				return GetClipboardDataFailed{};
			}
			auto saveDataOpt = getTarget(preset->outCommand);
			if (!saveDataOpt)
			{
				return GetClipboardDataFailed{};
			}
			return GetClipboardDataChanged{ std::move(*saveDataOpt) };
		}
	};

	std::unique_ptr<ClipboardImpl> ExternalClipboardFactory()
	{
		return std::make_unique<ExternalClipboardImpl>();
	}
}
