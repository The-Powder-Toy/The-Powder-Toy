#pragma once
#include <memory>
#include <optional>
#include "common/String.h"

class GameSave;

namespace Clipboard
{
	const ByteString clipboardFormatName = "application/vnd.powdertoy.save";
	void SetClipboardData(std::unique_ptr<GameSave> data);
	const GameSave *GetClipboardData();
	void Init();
	bool GetEnabled();
	void SetEnabled(bool newEnabled);
	void RecreateWindow();
	std::optional<String> Explanation();
}
