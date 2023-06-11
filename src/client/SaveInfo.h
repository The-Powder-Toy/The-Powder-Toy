#pragma once
#include "common/String.h"
#include <list>
#include <memory>
#include <ctime>

#ifdef GetUserName
# undef GetUserName // dammit windows
#endif

class GameSave;

class SaveInfo
{
private:
public:
	int id;
	time_t createdDate;
	time_t updatedDate;
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
	std::unique_ptr<GameSave> gameSave;

	SaveInfo(int _id, time_t _createdDate, time_t _updatedDate, int _votesUp, int _votesDown, ByteString _userName, String _name);

	SaveInfo(int _id, time_t _createdDate, time_t _updatedDate, int _votesUp, int _votesDown, int _vote, ByteString _userName, String _name, String description_, bool published_, std::list<ByteString> tags);

	void SetName(String name);
	const String &GetName() const;

	void SetDescription(String description);
	const String &GetDescription() const;

	void SetPublished(bool published);
	bool GetPublished() const;

	void SetUserName(ByteString userName);
	const ByteString &GetUserName() const;

	void SetID(int id);
	int GetID() const;

	void SetVote(int vote);
	int GetVote() const;

	void SetVotesUp(int votesUp);
	int GetVotesUp() const;

	void SetVotesDown(int votesDown);
	int GetVotesDown() const;

	void SetVersion(int version);
	int GetVersion() const;

	void SetTags(std::list<ByteString> tags);
	std::list<ByteString> GetTags() const;

	const GameSave *GetGameSave() const;
	std::unique_ptr<GameSave> TakeGameSave();
	void SetGameSave(std::unique_ptr<GameSave> newGameSave);

	std::unique_ptr<SaveInfo> CloneInfo() const;
};
