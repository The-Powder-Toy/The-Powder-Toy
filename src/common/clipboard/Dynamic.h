#pragma once
#include "common/String.h"
#include <vector>
#include <variant>
#include <memory>

class GameSave;

namespace Clipboard
{
	class ClipboardImpl
	{
	public:
		virtual ~ClipboardImpl() = default;

		virtual void SetClipboardData() = 0;

		struct GetClipboardDataUnchanged
		{
		};
		struct GetClipboardDataChanged
		{
			std::vector<char> data;
		};
		struct GetClipboardDataFailed
		{
		};
		struct GetClipboardDataUnknown
		{
		};
		using GetClipboardDataResult = std::variant<
			GetClipboardDataUnchanged,
			GetClipboardDataChanged,
			GetClipboardDataFailed,
			GetClipboardDataUnknown
		>;
		virtual GetClipboardDataResult GetClipboardData() = 0;
	};

	extern std::unique_ptr<GameSave> clipboardData;

	void SerializeClipboard(std::vector<char> &saveData);

	extern int currentSubsystem;
}
