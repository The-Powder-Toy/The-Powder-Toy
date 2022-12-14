#ifndef SAVEFILE_H_
#define SAVEFILE_H_

#include "common/String.h"

class GameSave;

class SaveFile {
public:
	SaveFile(SaveFile & save);
	SaveFile(ByteString filename, bool newLazyLoad = false);

	GameSave * GetGameSave();
	void SetGameSave(GameSave * save);
	String GetDisplayName();
	void SetDisplayName(String displayName);
	ByteString GetName();
	void SetFileName(ByteString fileName);
	String GetError();
	void SetLoadingError(String error);

	void LazyUnload();

	virtual ~SaveFile();
private:
	GameSave * gameSave;
	ByteString filename;
	String displayName;
	String loadingError;
	bool lazyLoad;
};

#endif /* SAVEFILE_H_ */
