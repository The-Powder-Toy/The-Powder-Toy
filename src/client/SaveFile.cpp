#include "SaveFile.h"
#include "GameSave.h"
#include "common/platform/Platform.h"

SaveFile::SaveFile(ByteString filename, bool newLazyLoad):
	filename(filename),
	displayName(filename.FromUtf8()),
	loadingError(""),
	lazyLoad(newLazyLoad)
{

}

const GameSave *SaveFile::LazyGetGameSave() // non-owning
{
	if (!gameSave && !loadingError.size() && lazyLoad)
	{
		try
		{
			std::vector<char> data;
			if (Platform::ReadFile(data, filename))
			{
				gameSave = std::make_unique<GameSave>(std::move(data));
			}
			else
			{
				loadingError = "cannot access file";
			}
		}
		catch(std::exception & e)
		{
			loadingError = ByteString(e.what()).FromUtf8();
		}
	}
	return gameSave.get();
}

const GameSave *SaveFile::GetGameSave() const
{
	return gameSave.get();
}

std::unique_ptr<GameSave> SaveFile::TakeGameSave()
{
	return std::move(gameSave);
}

void SaveFile::LazyUnload()
{
	if (lazyLoad)
	{
		gameSave.reset();
	}
}

void SaveFile::SetGameSave(std::unique_ptr<GameSave> newGameSave)
{
	gameSave = std::move(newGameSave);
}

const ByteString &SaveFile::GetName() const
{
	return filename;
}

void SaveFile::SetFileName(ByteString fileName)
{
	this->filename = fileName;
}

const String &SaveFile::GetDisplayName() const
{
	return displayName;
}

void SaveFile::SetDisplayName(String displayName)
{
	this->displayName = displayName;
}

const String &SaveFile::GetError() const
{
	return loadingError;
}

void SaveFile::SetLoadingError(String error)
{
	loadingError = error;
}
