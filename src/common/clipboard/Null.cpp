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

	bool GetEnabled()
	{
		return false;
	}

	void SetEnabled(bool)
	{
	}

	void RecreateWindow()
	{
	}
}
