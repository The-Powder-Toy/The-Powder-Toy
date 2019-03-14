#include <cmath>
#include "PreviewModel.h"
#include "Format.h"
#include "client/Client.h"
#include "client/GameSave.h"
#include "common/tpt-minmax.h"
#include "gui/dialogues/ErrorMessage.h"
#include "PreviewModelException.h"

PreviewModel::PreviewModel():
	doOpen(false),
	canOpen(true),
	saveInfo(NULL),
	saveData(NULL),
	saveComments(NULL),
	saveDataDownload(NULL),
	commentsDownload(NULL),
	commentBoxEnabled(false),
	commentsLoaded(false),
	commentsTotal(0),
	commentsPageNumber(1)
{

}

void PreviewModel::SetFavourite(bool favourite)
{
	if (saveInfo)
	{
		if (Client::Ref().FavouriteSave(saveInfo->id, favourite) == RequestOkay)
			saveInfo->Favourite = favourite;
		else if (favourite)
			throw PreviewModelException("Error, could not fav. the save: " + Client::Ref().GetLastError());
		else
			throw PreviewModelException("Error, could not unfav. the save: " + Client::Ref().GetLastError());
		notifySaveChanged();
	}
}

bool PreviewModel::GetCommentBoxEnabled()
{
	return commentBoxEnabled;
}

void PreviewModel::SetCommentBoxEnabled(bool enabledState)
{
	if(enabledState != commentBoxEnabled)
	{
		commentBoxEnabled = enabledState;
		notifyCommentBoxEnabledChanged();
	}
}

void PreviewModel::UpdateSave(int saveID, int saveDate)
{
	this->saveID = saveID;
	this->saveDate = saveDate;

	if (saveInfo)
	{
		delete saveInfo;
		saveInfo = NULL;
	}
	if (saveData)
	{
		delete saveData;
		saveData = NULL;
	}
	ClearComments();
	notifySaveChanged();
	notifySaveCommentsChanged();

	ByteString url;
	if (saveDate)
		url = ByteString::Build(STATICSCHEME, STATICSERVER, "/", saveID, "_", saveDate, ".cps");
	else
		url = ByteString::Build(STATICSCHEME, STATICSERVER, "/", saveID, ".cps");
	saveDataDownload = new http::Request(url);
	saveDataDownload->Start();

	url = ByteString::Build(SCHEME, SERVER , "/Browse/View.json?ID=", saveID);
	if (saveDate)
		url += ByteString::Build("&Date=", saveDate);
	saveInfoDownload = new http::Request(url);
	saveInfoDownload->AuthHeaders(ByteString::Build(Client::Ref().GetAuthUser().UserID), Client::Ref().GetAuthUser().SessionID);
	saveInfoDownload->Start();

	if (!GetDoOpen())
	{
		commentsLoaded = false;

		url = ByteString::Build(SCHEME, SERVER, "/Browse/Comments.json?ID=", saveID, "&Start=", (commentsPageNumber-1)*20, "&Count=20");
		commentsDownload = new http::Request(url);
		commentsDownload->AuthHeaders(ByteString::Build(Client::Ref().GetAuthUser().UserID), Client::Ref().GetAuthUser().SessionID);
		commentsDownload->Start();
	}
}

void PreviewModel::SetDoOpen(bool doOpen)
{
	this->doOpen = doOpen;
}

bool PreviewModel::GetDoOpen()
{
	return doOpen;
}

bool PreviewModel::GetCanOpen()
{
	return canOpen;
}

SaveInfo * PreviewModel::GetSaveInfo()
{
	return saveInfo;
}

int PreviewModel::GetCommentsPageNum()
{
	return commentsPageNumber;
}

int PreviewModel::GetCommentsPageCount()
{
	return std::max(1, (int)(ceil(commentsTotal/20.0f)));
}

bool PreviewModel::GetCommentsLoaded()
{
	return commentsLoaded;
}

void PreviewModel::UpdateComments(int pageNumber)
{
	if (commentsLoaded)
	{
		commentsLoaded = false;
		ClearComments();

		commentsPageNumber = pageNumber;
		if (!GetDoOpen())
		{
			ByteString url = ByteString::Build(SCHEME, SERVER, "/Browse/Comments.json?ID=", saveID, "&Start=", (commentsPageNumber-1)*20, "&Count=20");
			commentsDownload = new http::Request(url);
			commentsDownload->AuthHeaders(ByteString::Build(Client::Ref().GetAuthUser().UserID), Client::Ref().GetAuthUser().SessionID);
			commentsDownload->Start();
		}

		notifySaveCommentsChanged();
		notifyCommentsPageChanged();
	}
}

void PreviewModel::CommentAdded()
{
	if (saveInfo)
		saveInfo->Comments++;
	commentsTotal++;
}

void PreviewModel::OnSaveReady()
{
	commentsTotal = saveInfo->Comments;
	try
	{
		GameSave *gameSave = new GameSave(*saveData);
		if (gameSave->fromNewerVersion)
			new ErrorMessage("This save is from a newer version", "Please update TPT in game or at https://powdertoy.co.uk");
		saveInfo->SetGameSave(gameSave);
	}
	catch(ParseException &e)
	{
		new ErrorMessage("Error", ByteString(e.what()).FromUtf8());
		canOpen = false;
	}
	notifySaveChanged();
	notifyCommentsPageChanged();
	//make sure author name comments are red
	if (commentsLoaded)
		notifySaveCommentsChanged();
}

void PreviewModel::ClearComments()
{
	if (saveComments)
	{
		for (size_t i = 0; i < saveComments->size(); i++)
			delete saveComments->at(i);
		saveComments->clear();
		delete saveComments;
		saveComments = NULL;
	}
}

bool PreviewModel::ParseSaveInfo(ByteString &saveInfoResponse)
{
	delete saveInfo;

	try
	{
		std::istringstream dataStream(saveInfoResponse);
		Json::Value objDocument;
		dataStream >> objDocument;

		int tempID = objDocument["ID"].asInt();
		int tempScoreUp = objDocument["ScoreUp"].asInt();
		int tempScoreDown = objDocument["ScoreDown"].asInt();
		int tempMyScore = objDocument["ScoreMine"].asInt();
		ByteString tempUsername = objDocument["Username"].asString();
		String tempName = ByteString(objDocument["Name"].asString()).FromUtf8();
		String tempDescription = ByteString(objDocument["Description"].asString()).FromUtf8();
		int tempCreatedDate = objDocument["DateCreated"].asInt();
		int tempUpdatedDate = objDocument["Date"].asInt();
		bool tempPublished = objDocument["Published"].asBool();
		bool tempFavourite = objDocument["Favourite"].asBool();
		int tempComments = objDocument["Comments"].asInt();
		int tempViews = objDocument["Views"].asInt();
		int tempVersion = objDocument["Version"].asInt();

		Json::Value tagsArray = objDocument["Tags"];
		std::list<ByteString> tempTags;
		for (Json::UInt j = 0; j < tagsArray.size(); j++)
			tempTags.push_back(tagsArray[j].asString());

		saveInfo = new SaveInfo(tempID, tempCreatedDate, tempUpdatedDate, tempScoreUp,
		                        tempScoreDown, tempMyScore, tempUsername, tempName,
		                        tempDescription, tempPublished, tempTags);
		saveInfo->Comments = tempComments;
		saveInfo->Favourite = tempFavourite;
		saveInfo->Views = tempViews;
		saveInfo->Version = tempVersion;

		// This is a workaround for a bug on the TPT server where the wrong 404 save is returned
		// Redownload the .cps file for a fixed version of the 404 save
		if (tempID == 404 && this->saveID != 404)
		{
			if (saveDataDownload)
				saveDataDownload->Cancel();
			delete saveData;
			saveData = NULL;
			saveDataDownload = new http::Request(ByteString::Build(STATICSCHEME, STATICSERVER, "/2157797.cps"));
			saveDataDownload->Start();
		}
		return true;
	}
	catch (std::exception &e)
	{
		saveInfo = NULL;
		return false;
	}
}

bool PreviewModel::ParseComments(ByteString &commentsResponse)
{
	ClearComments();
	saveComments = new std::vector<SaveComment*>();
	try
	{
		std::istringstream dataStream(commentsResponse);
		Json::Value commentsArray;
		dataStream >> commentsArray;

		for (Json::UInt j = 0; j < commentsArray.size(); j++)
		{
			int userID = ByteString(commentsArray[j]["UserID"].asString()).ToNumber<int>();
			ByteString username = commentsArray[j]["Username"].asString();
			ByteString formattedUsername = commentsArray[j]["FormattedUsername"].asString();
			if (formattedUsername == "jacobot")
				formattedUsername = "\bt" + formattedUsername;
			String comment = ByteString(commentsArray[j]["Text"].asString()).FromUtf8();
			saveComments->push_back(new SaveComment(userID, username, formattedUsername, comment));
		}
		return true;
	}
	catch (std::exception &e)
	{
		return false;
	}
}

void PreviewModel::Update()
{
	if (saveDataDownload && saveDataDownload->CheckDone())
	{
		int status;
		ByteString ret = saveDataDownload->Finish(&status);

		ByteString nothing;
		Client::Ref().ParseServerReturn(nothing, status, true);
		if (status == 200 && ret.size())
		{
			delete saveData;
			saveData = new std::vector<unsigned char>(ret.begin(), ret.end());
			if (saveInfo && saveData)
				OnSaveReady();
		}
		else
		{
			for (size_t i = 0; i < observers.size(); i++)
			{
				observers[i]->SaveLoadingError(Client::Ref().GetLastError());
			}
		}
		saveDataDownload = NULL;
	}

	if (saveInfoDownload && saveInfoDownload->CheckDone())
	{
		int status;
		ByteString ret = saveInfoDownload->Finish(&status);

		ByteString nothing;
		Client::Ref().ParseServerReturn(nothing, status, true);
		if (status == 200 && ret.size())
		{
			if (ParseSaveInfo(ret))
			{
				if (saveInfo && saveData)
					OnSaveReady();
			}
			else
			{
				for (size_t i = 0; i < observers.size(); i++)
					observers[i]->SaveLoadingError("Could not parse save info");
			}
		}
		else
		{
			for (size_t i = 0; i < observers.size(); i++)
				observers[i]->SaveLoadingError(Client::Ref().GetLastError());
		}
		saveInfoDownload = NULL;
	}

	if (commentsDownload && commentsDownload->CheckDone())
	{
		int status;
		ByteString ret = commentsDownload->Finish(&status);
		ClearComments();

		ByteString nothing;
		Client::Ref().ParseServerReturn(nothing, status, true);
		if (status == 200 && ret.size())
			ParseComments(ret);

		commentsLoaded = true;
		notifySaveCommentsChanged();
		notifyCommentsPageChanged();

		commentsDownload = NULL;
	}
}

std::vector<SaveComment*> * PreviewModel::GetComments()
{
	return saveComments;
}

void PreviewModel::notifySaveChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifySaveChanged(this);
	}
}

void PreviewModel::notifyCommentBoxEnabledChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyCommentBoxEnabledChanged(this);
	}
}

void PreviewModel::notifyCommentsPageChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyCommentsPageChanged(this);
	}
}

void PreviewModel::notifySaveCommentsChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyCommentsChanged(this);
	}
}

void PreviewModel::AddObserver(PreviewView * observer)
{
	observers.push_back(observer);
	observer->NotifySaveChanged(this);
	observer->NotifyCommentsChanged(this);
	observer->NotifyCommentsPageChanged(this);
	observer->NotifyCommentBoxEnabledChanged(this);
}


PreviewModel::~PreviewModel()
{
	if (saveDataDownload)
		saveDataDownload->Cancel();
	if (saveInfoDownload)
		saveInfoDownload->Cancel();
	if (commentsDownload)
		commentsDownload->Cancel();
	delete saveInfo;
	delete saveData;
	ClearComments();
}
