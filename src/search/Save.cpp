/*
 * Save.cpp
 *
 *  Created on: Jan 26, 2012
 *      Author: Simon
 */

#include "Save.h"
#include "client/Client.h"

Save::Save(Save & save) :
		userName(save.userName), name(save.name), Description(save.Description), date(
				save.date), Published(save.Published), id(save.id), votesUp(
				save.votesUp), votesDown(save.votesDown), data(NULL), vote(save.vote), tags(save.tags) {
	if (save.data) {
		data = (unsigned char *) malloc(save.dataLength);
		memcpy(data, save.data, save.dataLength);
		dataLength = save.dataLength;
	}
}

Save::Save(int _id, int _date, int _votesUp, int _votesDown, string _userName,
		string _name) :
		id(_id), votesUp(_votesUp), votesDown(_votesDown), userName(_userName), name(
				_name), Description("No description provided"), date(_date), Published(
				true), data(NULL), vote(0), tags() {
}

Save::Save(int _id, int date_, int _votesUp, int _votesDown, int _vote, string _userName,
		string _name, string description_, bool published_, vector<string> tags_) :
		id(_id), votesUp(_votesUp), votesDown(_votesDown), userName(_userName), name(
				_name), Description(description_), date(date_), Published(
				published_), data(NULL), vote(_vote), tags(tags_) {
}

Save::~Save()
{
	if(data)
	{
		free(data);
	}
}

void Save::SetName(string name) {
	this->name = name;
}
string Save::GetName() {
	return name;
}

void Save::SetVote(int vote)
{
	this->vote = vote;
}
int Save::GetVote()
{
	return vote;
}

void Save::SetUserName(string userName) {
	this->userName = userName;
}
string Save::GetUserName() {
	return userName;
}

void Save::SetID(int id) {
	this->id = id;
}
int Save::GetID() {
	return id;
}

void Save::SetVotesUp(int votesUp) {
	this->votesUp = votesUp;
}
int Save::GetVotesUp() {
	return votesUp;
}

void Save::SetVotesDown(int votesDown) {
	this->votesDown = votesDown;
}
int Save::GetVotesDown() {
	return votesDown;
}

void Save::SetTags(vector<string> tags)
{
	this->tags = tags;
}
vector<string> Save::GetTags()
{
	return tags;
}

unsigned char * Save::GetData() {
	if (!data) {
		data = Client::Ref().GetSaveData(id, date, dataLength);
	}
	return data;
}
void Save::SetData(unsigned char * data_, int dataLength) {
	if(data)
		free(data);
	data = data_;
	this->dataLength = dataLength;
}

int Save::GetDataLength() {
	if (!data) {
		data = Client::Ref().GetSaveData(id, date, dataLength);
	}
	return dataLength;
}
