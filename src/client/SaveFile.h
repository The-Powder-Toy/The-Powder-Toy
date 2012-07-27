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

class Thumbnail;
class SaveFile {
public:
	SaveFile(SaveFile & save);
	SaveFile(string filename);

	Thumbnail * GetThumbnail();
	GameSave * GetGameSave();
	void SetThumbnail(Thumbnail * thumb);
	void SetGameSave(GameSave * save);
	string GetName();

	virtual ~SaveFile();
private:
	Thumbnail * thumbnail;
	GameSave * gameSave;
	string filename;
};

#endif /* SAVEFILE_H_ */
