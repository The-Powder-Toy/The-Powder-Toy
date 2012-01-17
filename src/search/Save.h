#ifndef SAVE_H
#define SAVE_H

#include <string>

using namespace std;

class Save
{
private:
	int id;
	int votesUp, votesDown;
	string userName;
	string name;
public:
	Save(int _id, int _votesUp, int _votesDown, string _userName, string _name):
		id(_id),
		votesUp(_votesUp),
		votesDown(_votesDown),
		userName(_userName),
		name(_name)
	{
	}

	void SetName(string name){ this->name = name; }
	string GetName(){ return name; }

	void SetUserName(string userName){ this->userName = userName; }
	string GetUserName(){ return userName; }

	void SetID(int id){ this->id = id; }
	int GetID(){ return id; }

	void SetVotesUp(int votesUp){ this->votesUp = votesUp; }
	int GetVotesUp(){ return votesUp; }

	void SetVotesDown(int votesDown){ this->votesDown = votesDown; }
	int GetVotesDown(){ return votesDown; }
};

#endif // SAVE_H
