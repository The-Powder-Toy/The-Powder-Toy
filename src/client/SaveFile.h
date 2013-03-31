#ifndef SAVEFILE_H_
#define SAVEFILE_H_

#include <string>

class GameSave;
class Thumbnail;

class SaveFile {
public:
	SaveFile(SaveFile & save);
	SaveFile(std::string filename);

	Thumbnail * GetThumbnail();
	GameSave * GetGameSave();
	void SetThumbnail(Thumbnail * thumb);
	void SetGameSave(GameSave * save);
	std::string GetDisplayName();
	void SetDisplayName(std::string displayName);
	std::string GetName();
	void SetFileName(std::string fileName);

	virtual ~SaveFile();
private:
	Thumbnail * thumbnail;
	GameSave * gameSave;
	std::string filename;
	std::string displayName;
};

#endif /* SAVEFILE_H_ */
