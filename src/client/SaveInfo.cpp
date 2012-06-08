/*
 * Save.cpp
 *
 *  Created on: Jan 26, 2012
 *      Author: Simon
 */

#include "client/SaveInfo.h"
#include "client/Client.h"

SaveInfo::SaveInfo(SaveInfo & save) :
		userName(save.userName), name(save.name), Description(save.Description), date(
				save.date), Published(save.Published), id(save.id), votesUp(
				save.votesUp), votesDown(save.votesDown), gameSave(NULL), vote(save.vote), tags(save.tags) {
	if(save.gameSave)
		gameSave = new GameSave(*save.gameSave);
}

SaveInfo::SaveInfo(int _id, int _date, int _votesUp, int _votesDown, string _userName,
		string _name) :
		id(_id), votesUp(_votesUp), votesDown(_votesDown), userName(_userName), name(
				_name), Description("No description provided"), date(_date), Published(
				true), gameSave(NULL), vote(0), tags() {
}

SaveInfo::SaveInfo(int _id, int date_, int _votesUp, int _votesDown, int _vote, string _userName,
		string _name, string description_, bool published_, vector<string> tags_) :
		id(_id), votesUp(_votesUp), votesDown(_votesDown), userName(_userName), name(
				_name), Description(description_), date(date_), Published(
				published_), gameSave(NULL), vote(_vote), tags(tags_) {
}

SaveInfo::~SaveInfo()
{
	if(gameSave)
	{
		delete gameSave;
	}
}

void SaveInfo::SetName(string name) {
	this->name = name;
}
string SaveInfo::GetName() {
	return name;
}

void SaveInfo::SetVote(int vote)
{
	this->vote = vote;
}
int SaveInfo::GetVote()
{
	return vote;
}

void SaveInfo::SetUserName(string userName) {
	this->userName = userName;
}

string SaveInfo::GetUserName() {
	return userName;
}

void SaveInfo::SetID(int id) {
	this->id = id;
}
int SaveInfo::GetID() {
	return id;
}

void SaveInfo::SetVotesUp(int votesUp) {
	this->votesUp = votesUp;
}
int SaveInfo::GetVotesUp() {
	return votesUp;
}

void SaveInfo::SetVotesDown(int votesDown) {
	this->votesDown = votesDown;
}
int SaveInfo::GetVotesDown() {
	return votesDown;
}

void SaveInfo::SetTags(vector<string> tags)
{
	this->tags = tags;
}
vector<string> SaveInfo::GetTags()
{
	return tags;
}

GameSave * SaveInfo::GetGameSave()
{
	return gameSave;
}

void SaveInfo::SetGameSave(GameSave * saveGame)
{
	if(gameSave)
		delete gameSave;
	gameSave = saveGame;
}
