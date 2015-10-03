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

SaveFile::SaveFile(std::string filename):
	thumbnail(NULL),
	gameSave(NULL),
	filename(filename),
	displayName(filename),
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

std::string SaveFile::GetName()
{
	return filename;
}

void SaveFile::SetFileName(std::string fileName)
{
	this->filename = fileName;
}

std::string SaveFile::GetDisplayName()
{
	return displayName;
}

void SaveFile::SetDisplayName(std::string displayName)
{
	this->displayName = displayName;
}

std::string SaveFile::GetError()
{
	return loadingError;
}

void SaveFile::SetLoadingError(std::string error)
{
	loadingError = error;
}

SaveFile::~SaveFile() {
	delete gameSave;
	delete thumbnail;
}

