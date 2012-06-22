#ifndef SAVE_H
#define SAVE_H

#include <vector>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include "GameSave.h"

using namespace std;

class SaveInfo
{
private:
public:
	int id;
	int date;
	int votesUp, votesDown;
	bool Favourite;
	int Comments;

	GameSave * gameSave;

	SaveInfo(SaveInfo & save);

	SaveInfo(int _id, int _date, int _votesUp, int _votesDown, string _userName, string _name);

	SaveInfo(int _id, int date_, int _votesUp, int _votesDown, int _vote, string _userName, string _name, string description_, bool published_, vector<string> tags);

	~SaveInfo();

	string userName;
	string name;

	string Description;

	vector<string> tags;

	int vote;

	bool Published;

	void SetName(string name);
	string GetName();

	void SetUserName(string userName);
	string GetUserName();

	void SetID(int id);
	int GetID();

	void SetVote(int vote);
	int GetVote();

	void SetVotesUp(int votesUp);
	int GetVotesUp();

	void SetVotesDown(int votesDown);
	int GetVotesDown();

	void SetTags(vector<string> tags);
	vector<string> GetTags();

	GameSave * GetGameSave();
	void SetGameSave(GameSave * gameSave);
};

#endif // SAVE_H
