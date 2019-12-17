#ifndef SAVE_H
#define SAVE_H
#include "Config.h"

#include <list>
#include "common/String.h"

#ifdef GetUserName
# undef GetUserName // dammit windows
#endif

class GameSave;

class SaveInfo
{
private:
public:
	int id;
	int createdDate;
	int updatedDate;
	int votesUp, votesDown;
	int vote;
	bool Favourite;
	int Comments;
	int Views;
	int Version;

	ByteString userName;

	String name;
	String Description;
	bool Published;

	std::list<ByteString> tags;
	GameSave * gameSave;

	SaveInfo(SaveInfo & save);

	SaveInfo(int _id, int _createdDate, int _updatedDate, int _votesUp, int _votesDown, ByteString _userName, String _name);

	SaveInfo(int _id, int _createdDate, int _updatedDate, int _votesUp, int _votesDown, int _vote, ByteString _userName, String _name, String description_, bool published_, std::list<ByteString> tags);

	~SaveInfo();

	void SetName(String name);
	String GetName();

	void SetDescription(String description);
	String GetDescription();

	void SetPublished(bool published);
	bool GetPublished();

	void SetUserName(ByteString userName);
	ByteString GetUserName();

	void SetID(int id);
	int GetID();

	void SetVote(int vote);
	int GetVote();

	void SetVotesUp(int votesUp);
	int GetVotesUp();

	void SetVotesDown(int votesDown);
	int GetVotesDown();

	void SetVersion(int version);
	int GetVersion();

	void SetTags(std::list<ByteString> tags);
	std::list<ByteString> GetTags();

	GameSave * GetGameSave();
	void SetGameSave(GameSave * gameSave);
};

#endif // SAVE_H
