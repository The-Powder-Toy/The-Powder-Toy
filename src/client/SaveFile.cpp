/*
 * SaveFile.cpp
 *
 *  Created on: Jun 6, 2012
 *      Author: Simon
 */

#include "SaveFile.h"

SaveFile::SaveFile(SaveFile & save):
	gameSave(NULL)
{
	if(save.gameSave)
		gameSave = new GameSave(*save.gameSave);
}

SaveFile::SaveFile(string filename):
		filename(filename),
		gameSave(NULL)
	{
	//Load file
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

SaveFile::~SaveFile() {
	if(gameSave)
		delete gameSave;
}

