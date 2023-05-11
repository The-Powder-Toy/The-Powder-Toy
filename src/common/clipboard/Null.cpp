#include "Clipboard.h"
#include "client/GameSave.h"

namespace Clipboard
{
	void SetClipboardData(std::unique_ptr<GameSave> data)
	{
	}

	const GameSave *GetClipboardData()
	{
		return nullptr;
	}

	void Init()
	{
	}

	void RecreateWindow()
	{
	}
}
