#ifndef SAVE_H
#define SAVE_H

#include <list>
#include <vector>
#include <string>
#include <cstdlib>
#include <iostream>

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

	std::string userName;

	std::string name;
	std::string Description;
	bool Published;

	std::list<std::string> tags;
	GameSave * gameSave;

	SaveInfo(SaveInfo & save);

	SaveInfo(int _id, int _createdDate, int _updatedDate, int _votesUp, int _votesDown, std::string _userName, std::string _name);

	SaveInfo(int _id, int _createdDate, int _updatedDate, int _votesUp, int _votesDown, int _vote, std::string _userName, std::string _name, std::string description_, bool published_, std::list<std::string> tags);

	~SaveInfo();

	void SetName(std::string name);
	std::string GetName();

	void SetDescription(std::string description);
	std::string GetDescription();

	void SetPublished(bool published);
	bool GetPublished();

	void SetUserName(std::string userName);
	std::string GetUserName();

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

	void SetTags(std::list<std::string> tags);
	std::list<std::string> GetTags();

	GameSave * GetGameSave();
	void SetGameSave(GameSave * gameSave);
};

#endif // SAVE_H
