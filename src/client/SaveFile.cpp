#include "SaveFile.h"
#include "GameSave.h"
#include "common/Platform.h"

SaveFile::SaveFile(SaveFile & save):
	gameSave(NULL),
	filename(save.filename),
	displayName(save.displayName),
	loadingError(save.loadingError),
	lazyLoad(save.lazyLoad)
{
	if (save.gameSave)
		gameSave = new GameSave(*save.gameSave);
}

SaveFile::SaveFile(ByteString filename, bool newLazyLoad):
	gameSave(NULL),
	filename(filename),
	displayName(filename.FromUtf8()),
	loadingError(""),
	lazyLoad(newLazyLoad)
{

}

GameSave * SaveFile::GetGameSave()
{
	if (!gameSave && !loadingError.size() && lazyLoad)
	{
		try
		{
			std::vector<char> data;
			if (Platform::ReadFile(data, filename))
			{
				gameSave = new GameSave(std::move(data));
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
	return gameSave;
}

void SaveFile::LazyUnload()
{
	if (lazyLoad && gameSave)
	{
		delete gameSave;
		gameSave = nullptr;
	}
}

void SaveFile::SetGameSave(GameSave * save)
{
	gameSave = save;
}

ByteString SaveFile::GetName()
{
	return filename;
}

void SaveFile::SetFileName(ByteString fileName)
{
	this->filename = fileName;
}

String SaveFile::GetDisplayName()
{
	return displayName;
}

void SaveFile::SetDisplayName(String displayName)
{
	this->displayName = displayName;
}

String SaveFile::GetError()
{
	return loadingError;
}

void SaveFile::SetLoadingError(String error)
{
	loadingError = error;
}

SaveFile::~SaveFile() {
	if (gameSave)
	{
		delete gameSave;
	}
}
