#include "SaveFile.h"
#include "GameSave.h"
#include "Client.h"
#include "gui/search/Thumbnail.h"

SaveFile::SaveFile(SaveFile & save):
	thumbnail(NULL),
	gameSave(NULL),
	filename(save.filename),
	displayName(save.displayName),
	loadingError(save.loadingError)
{
	if (save.gameSave)
		gameSave = new GameSave(*save.gameSave);
	if (save.thumbnail)
		thumbnail = new Thumbnail(*save.thumbnail);
}

Thumbnail * SaveFile::GetThumbnail()
{
	return thumbnail;
}

void SaveFile::SetThumbnail(Thumbnail * thumb)
{
	thumbnail = thumb;
}

SaveFile::SaveFile(ByteString filename):
	thumbnail(NULL),
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
	delete thumbnail;
}

