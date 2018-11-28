#include "SaveFile.h"
#include "GameSave.h"
#include "Client.h"

SaveFile::SaveFile(SaveFile & save):
	gameSave(NULL),
	filename(save.filename),
	displayName(save.displayName),
	loadingError(save.loadingError)
{
	if (save.gameSave)
		gameSave = new GameSave(*save.gameSave);
}

SaveFile::SaveFile(ByteString filename):
	gameSave(NULL),
	filename(filename),
	displayName(filename.FromUtf8()),
	loadingError("")
{

}

GameSave * SaveFile::GetGameSave()
{
	return gameSave;
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
	delete gameSave;
}

