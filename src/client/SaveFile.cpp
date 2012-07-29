/*
 * SaveFile.cpp
 *
 *  Created on: Jun 6, 2012
 *      Author: Simon
 */

#include "SaveFile.h"
#include "Client.h"
 #include "search/Thumbnail.h"

SaveFile::SaveFile(SaveFile & save):
	gameSave(NULL),
	thumbnail(NULL),
	filename(save.filename),
	displayName(save.displayName)
{
	if(save.gameSave)
		gameSave = new GameSave(*save.gameSave);
	if(save.thumbnail)
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

SaveFile::SaveFile(string filename):
		filename(filename),
		displayName(filename),
		gameSave(NULL),
		thumbnail(NULL)
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

string SaveFile::GetName()
{
	return filename;
}

string SaveFile::GetDisplayName()
{
	return displayName;
}

void SaveFile::SetDisplayName(string displayName)
{
	this->displayName = displayName;
}

SaveFile::~SaveFile() {
	if(gameSave)
		delete gameSave;
	if(thumbnail)
		delete thumbnail;
}

