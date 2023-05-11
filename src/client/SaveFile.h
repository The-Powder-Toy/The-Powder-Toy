#pragma once
#include "common/String.h"
#include <memory>

class GameSave;

class SaveFile {
public:
	SaveFile(ByteString filename, bool newLazyLoad = false);

	const GameSave *LazyGetGameSave();
	const GameSave *GetGameSave() const;
	std::unique_ptr<GameSave> TakeGameSave();
	void SetGameSave(std::unique_ptr<GameSave> newSameSave);
	const String &GetDisplayName() const;
	void SetDisplayName(String displayName);
	const ByteString &GetName() const;
	void SetFileName(ByteString fileName);
	const String &GetError() const;
	void SetLoadingError(String error);

	void LazyUnload();
private:
	std::unique_ptr<GameSave> gameSave;
	ByteString filename;
	String displayName;
	String loadingError;
	bool lazyLoad;
};
