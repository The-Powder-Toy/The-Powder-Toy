#include "SaveInfo.h"
#include "GameSave.h"
#include "Client.h"

SaveInfo::SaveInfo(SaveInfo & save):
	id(save.id),
	createdDate(save.createdDate),
	updatedDate(save.updatedDate),
	votesUp(save.votesUp),
	votesDown(save.votesDown),
	vote(save.vote),
	Favourite(false),
	Comments(save.Comments),
	Views(save.Views),
	Version(save.Version),
	userName(save.userName),
	name(save.name),
	Description(save.Description),
	Published(save.Published),
	gameSave(NULL)
{
	std::list<std::string> tagsSorted = save.tags;
	tagsSorted.sort();
	tags = tagsSorted;
	if (save.gameSave)
		gameSave = new GameSave(*save.gameSave);
}

SaveInfo::SaveInfo(int _id, int _createdDate, int _updatedDate, int _votesUp, int _votesDown, std::string _userName, std::string _name):
	id(_id),
	createdDate(_createdDate),
	updatedDate(_updatedDate),
	votesUp(_votesUp),
	votesDown(_votesDown),
	vote(0),
	Favourite(false),
	Comments(0),
	Views(0),
	Version(0),
	userName(_userName),
	name(_name),
	Description(""),
	Published(false),
	tags(),
	gameSave(NULL)
{

}

SaveInfo::SaveInfo(int _id, int _createdDate, int _updatedDate, int _votesUp, int _votesDown, int _vote, std::string _userName, std::string _name, std::string description_, bool published_, std::list<std::string> tags_):
	id(_id),
	createdDate(_createdDate),
	updatedDate(_updatedDate),
	votesUp(_votesUp),
	votesDown(_votesDown),
	vote(_vote),
	Favourite(false),
	Comments(0),
	Views(0),
	Version(0),
	userName(_userName),
	name(_name),
	Description(description_),
	Published(published_),
	tags(),
	gameSave(NULL)
{
	std::list<std::string> tagsSorted = tags_;
	tagsSorted.sort();
	tags=tagsSorted;
}

SaveInfo::~SaveInfo()
{
	if(gameSave)
	{
		delete gameSave;
	}
}

void SaveInfo::SetName(std::string name)
{
	this->name = name;
}
std::string SaveInfo::GetName()
{
	return name;
}

void SaveInfo::SetDescription(std::string description)
{
	Description = description;
}
std::string SaveInfo::GetDescription()
{
	return Description;
}

void SaveInfo::SetPublished(bool published)
{
	Published = published;
}
bool SaveInfo::GetPublished()
{
	return Published;
}

void SaveInfo::SetVote(int vote)
{
	this->vote = vote;
}
int SaveInfo::GetVote()
{
	return vote;
}

void SaveInfo::SetUserName(std::string userName)
{
	this->userName = userName;
}

std::string SaveInfo::GetUserName()
{
	return userName;
}

void SaveInfo::SetID(int id)
{
	this->id = id;
}
int SaveInfo::GetID()
{
	return id;
}

void SaveInfo::SetVotesUp(int votesUp)
{
	this->votesUp = votesUp;
}
int SaveInfo::GetVotesUp()
{
	return votesUp;
}

void SaveInfo::SetVotesDown(int votesDown)
{
	this->votesDown = votesDown;
}
int SaveInfo::GetVotesDown()
{
	return votesDown;
}

void SaveInfo::SetVersion(int version)
{
	this->Version = version;
}
int SaveInfo::GetVersion()
{
	return Version;
}

void SaveInfo::SetTags(std::list<std::string> tags)
{
	std::list<std::string> tagsSorted = tags;
	tagsSorted.sort();
	this->tags=tagsSorted;
}

std::list<std::string> SaveInfo::GetTags()
{
	return tags;
}

GameSave * SaveInfo::GetGameSave()
{
	return gameSave;
}

void SaveInfo::SetGameSave(GameSave * saveGame)
{
	delete gameSave;
	gameSave = saveGame;
}
