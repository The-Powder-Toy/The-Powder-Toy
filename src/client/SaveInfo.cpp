#include "SaveInfo.h"
#include "GameSave.h"

SaveInfo::SaveInfo(int _id, int _createdDate, int _updatedDate, int _votesUp, int _votesDown, ByteString _userName, String _name):
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
	Published(false)
{

}

SaveInfo::SaveInfo(int _id, int _createdDate, int _updatedDate, int _votesUp, int _votesDown, int _vote, ByteString _userName, String _name, String description_, bool published_, std::list<ByteString> tags_):
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
	Published(published_)
{
	std::list<ByteString> tagsSorted = tags_;
	tagsSorted.sort();
	tags=tagsSorted;
}

void SaveInfo::SetName(String name)
{
	this->name = name;
}
String SaveInfo::GetName()
{
	return name;
}

void SaveInfo::SetDescription(String description)
{
	Description = description;
}
String SaveInfo::GetDescription()
{
	return Description;
}

void SaveInfo::SetPublished(bool published)
{
	Published = published;
}
bool SaveInfo::GetPublished() const
{
	return Published;
}

void SaveInfo::SetVote(int vote)
{
	this->vote = vote;
}
int SaveInfo::GetVote() const
{
	return vote;
}

void SaveInfo::SetUserName(ByteString userName)
{
	this->userName = userName;
}

const ByteString &SaveInfo::GetUserName() const
{
	return userName;
}

void SaveInfo::SetID(int id)
{
	this->id = id;
}
int SaveInfo::GetID() const
{
	return id;
}

void SaveInfo::SetVotesUp(int votesUp)
{
	this->votesUp = votesUp;
}
int SaveInfo::GetVotesUp() const
{
	return votesUp;
}

void SaveInfo::SetVotesDown(int votesDown)
{
	this->votesDown = votesDown;
}
int SaveInfo::GetVotesDown() const
{
	return votesDown;
}

void SaveInfo::SetVersion(int version)
{
	this->Version = version;
}
int SaveInfo::GetVersion() const
{
	return Version;
}

void SaveInfo::SetTags(std::list<ByteString> tags)
{
	std::list<ByteString> tagsSorted = tags;
	tagsSorted.sort();
	this->tags=tagsSorted;
}

std::list<ByteString> SaveInfo::GetTags() const
{
	return tags;
}

const GameSave *SaveInfo::GetGameSave() const
{
	return gameSave.get();
}

std::unique_ptr<GameSave> SaveInfo::TakeGameSave()
{
	return std::move(gameSave);
}

void SaveInfo::SetGameSave(std::unique_ptr<GameSave> newGameSave)
{
	gameSave = std::move(newGameSave);
}

std::unique_ptr<SaveInfo> SaveInfo::CloneInfo() const
{
	auto clone = std::make_unique<SaveInfo>(id, createdDate, updatedDate, votesUp, votesDown, vote, userName, name, Description, Published, tags);
	clone->Favourite = false;
	clone->Comments = Comments;
	clone->Views = Views;
	clone->Version = Version;
	clone->tags.sort();
	return clone;
}
