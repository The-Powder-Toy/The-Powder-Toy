/*
 * SaveFile.h
 *
 *  Created on: Jun 6, 2012
 *      Author: Simon
 */

#ifndef SAVEFILE_H_
#define SAVEFILE_H_

#include <string>
#include "GameSave.h"

using namespace std;

class SaveFile {
public:
	SaveFile(SaveFile & save);
	SaveFile(string filename);

	GameSave * GetGameSave();
	void SetGameSave(GameSave * save);
	string GetName();

	virtual ~SaveFile();
private:
	GameSave * gameSave;
	string filename;
};

#endif /* SAVEFILE_H_ */
