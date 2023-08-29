#include "PreviewModel.h"
#include "client/http/GetSaveDataRequest.h"
#include "client/http/GetSaveRequest.h"
#include "client/http/GetCommentsRequest.h"
#include "client/http/FavouriteSaveRequest.h"
#include "Format.h"
#include "Misc.h"
#include "client/Client.h"
#include "client/GameSave.h"
#include "client/SaveInfo.h"
#include "gui/dialogues/ErrorMessage.h"
#include "PreviewView.h"
#include "Config.h"
#include <cmath>
#include <iostream>

constexpr auto commentsPerPage = 20;

void PreviewModel::SetFavourite(bool favourite)
{
	if (saveInfo)
	{
		queuedFavourite = favourite;
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

	saveInfo.reset();
	saveData.reset();
	saveComments.reset();
	notifySaveChanged();
	notifySaveCommentsChanged();

	saveDataDownload = std::make_unique<http::GetSaveDataRequest>(saveID, saveDate);
	saveDataDownload->Start();

	saveInfoDownload = std::make_unique<http::GetSaveRequest>(saveID, saveDate);
	saveInfoDownload->Start();

	if (!GetDoOpen())
	{
		commentsLoaded = false;

		commentsDownload = std::make_unique<http::GetCommentsRequest>(saveID, (commentsPageNumber - 1) * commentsPerPage, commentsPerPage);
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

void PreviewModel::SetFromUrl(bool fromUrl)
{
	this->fromUrl = fromUrl;
}

bool PreviewModel::GetFromUrl()
{
	return fromUrl;
}

bool PreviewModel::GetCanOpen()
{
	return canOpen;
}

const SaveInfo *PreviewModel::GetSaveInfo() const
{
	return saveInfo.get();
}

std::unique_ptr<SaveInfo> PreviewModel::TakeSaveInfo()
{
	return std::move(saveInfo);
}

int PreviewModel::GetCommentsPageNum()
{
	return commentsPageNumber;
}

int PreviewModel::GetCommentsPageCount()
{
	return std::max(1, ceilDiv(commentsTotal, commentsPerPage).first);
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
		saveComments.reset();

		commentsPageNumber = pageNumber;
		if (!GetDoOpen())
		{
			commentsDownload = std::make_unique<http::GetCommentsRequest>(saveID, (commentsPageNumber - 1) * commentsPerPage, commentsPerPage);
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
		auto gameSave = std::make_unique<GameSave>(*saveData);
		if (gameSave->fromNewerVersion)
			new ErrorMessage("This save is from a newer version", "Please update TPT in game or at https://powdertoy.co.uk");
		saveInfo->SetGameSave(std::move(gameSave));
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

void PreviewModel::Update()
{
	auto triggerOnSaveReady = false;
	if (saveDataDownload && saveDataDownload->CheckDone())
	{
		try
		{
			saveData = saveDataDownload->Finish();
			triggerOnSaveReady = true;
		}
		catch (const http::RequestError &ex)
		{
			auto why = ByteString(ex.what()).FromUtf8();
			for (size_t i = 0; i < observers.size(); i++)
			{
				observers[i]->SaveLoadingError(why);
			}
		}
		saveDataDownload.reset();
	}
	if (saveInfoDownload && saveInfoDownload->CheckDone())
	{
		try
		{
			saveInfo = saveInfoDownload->Finish();
			triggerOnSaveReady = true;
			// This is a workaround for a bug on the TPT server where the wrong 404 save is returned
			// Redownload the .cps file for a fixed version of the 404 save
			if (saveInfo->GetID() == 404 && saveID != 404)
			{
				saveData.reset();
				saveDataDownload = std::make_unique<http::GetSaveDataRequest>(2157797, 0);
				saveDataDownload->Start();
			}
		}
		catch (const http::RequestError &ex)
		{
			auto why = ByteString(ex.what()).FromUtf8();
			for (size_t i = 0; i < observers.size(); i++)
			{
				observers[i]->SaveLoadingError(why);
			}
		}
		saveInfoDownload.reset();
	}
	if (triggerOnSaveReady && saveInfo && saveData)
	{
		OnSaveReady();
	}

	if (commentsDownload && commentsDownload->CheckDone())
	{
		try
		{
			saveComments = commentsDownload->Finish();
		}
		catch (const http::RequestError &ex)
		{
			// TODO: handle
		}
		commentsLoaded = true;
		notifySaveCommentsChanged();
		notifyCommentsPageChanged();
		commentsDownload.reset();
	}

	if (favouriteSaveRequest && favouriteSaveRequest->CheckDone())
	{
		try
		{
			favouriteSaveRequest->Finish();
			if (saveInfo)
			{
				saveInfo->Favourite = favouriteSaveRequest->Favourite();
				notifySaveChanged();
			}
		}
		catch (const http::RequestError &ex)
		{
			if (favouriteSaveRequest->Favourite())
			{
				new ErrorMessage("Error", "Could not favourite the save: " + ByteString(ex.what()).FromUtf8());
			}
			else
			{
				new ErrorMessage("Error", "Could not unfavourite the save: " + ByteString(ex.what()).FromUtf8());
			}
		}
		favouriteSaveRequest.reset();
	}
	if (!favouriteSaveRequest && queuedFavourite)
	{
		if (saveInfo)
		{
			favouriteSaveRequest = std::make_unique<http::FavouriteSaveRequest>(saveInfo->id, *queuedFavourite);
			favouriteSaveRequest->Start();
		}
		queuedFavourite.reset();
	}
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
