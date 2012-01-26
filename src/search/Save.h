#ifndef SAVE_H
#define SAVE_H

#include <string>
#include <stdlib.h>
#include <iostream>
#include <string.h>

using namespace std;

class Save
{
private:
	int id;
	int date;
	int votesUp, votesDown;
	unsigned char * data;
	int dataLength;
public:
	Save(Save & save);

	Save(int _id, int _date, int _votesUp, int _votesDown, string _userName, string _name);

	Save(int _id, int date_, int _votesUp, int _votesDown, string _userName, string _name, string description_, bool published_);

	string userName;
	string name;

	string Description;

	bool Published;

	void SetName(string name);
	string GetName();

	void SetUserName(string userName);
	string GetUserName();

	void SetID(int id);
	int GetID();

	void SetVotesUp(int votesUp);
	int GetVotesUp();

	void SetVotesDown(int votesDown);
	int GetVotesDown();

	unsigned char * GetData();
	void SetData(unsigned char * data_);

	int GetDataLength();
};

#endif // SAVE_H
