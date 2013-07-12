#include <cmath>
#include "PreviewModel.h"
#include "client/Client.h"
#include "client/GameSave.h";
#include "PreviewModelException.h"

PreviewModel::PreviewModel():
	save(NULL),
	saveComments(NULL),
	saveData(NULL),
	doOpen(false),
	commentsTotal(0),
	commentsPageNumber(1),
	commentBoxEnabled(false),
	updateSaveDataInfo(NULL),
	updateSaveInfoInfo(NULL),
	updateSaveCommentsInfo(NULL)
{

}

void * PreviewModel::updateSaveInfoT(void * obj)
{
	SaveInfo * tempSave = Client::Ref().GetSave(((threadInfo*)obj)->saveID, ((threadInfo*)obj)->saveDate);
	((threadInfo*)obj)->threadFinished = true;
	if (((threadInfo*)obj)->previewExited)
	{
		if (tempSave)
			delete tempSave;
		delete obj;
	}
	return tempSave;
}

void * PreviewModel::updateSaveDataT(void * obj)
{
	int tempDataSize;
	unsigned char * tempData = Client::Ref().GetSaveData(((threadInfo*)obj)->saveID, ((threadInfo*)obj)->saveDate, tempDataSize);
	SaveData * tempSave = new SaveData(tempData, tempDataSize);
	((threadInfo*)obj)->threadFinished = true;
	if (((threadInfo*)obj)->previewExited)
	{
		if (tempSave)
			delete tempSave;
		if (tempData)
			free(tempData);
		delete obj;
	}
	return tempSave;
}

void * PreviewModel::updateSaveCommentsT(void * obj)
{
	std::vector<SaveComment*> * tempComments = Client::Ref().GetComments(((threadInfo*)obj)->saveID, (((threadInfo*)obj)->saveDate-1)*20, 20); //saveDate is used as commentsPageNumber
	((threadInfo*)obj)->threadFinished = true;
	if (((threadInfo*)obj)->previewExited)
	{
		if (tempComments)
		{
			for(int i = 0; i < tempComments->size(); i++)
				delete tempComments->at(i);
			tempComments->clear();
			delete tempComments;
		}
		delete obj;
	}
	return tempComments;
}

void PreviewModel::SetFavourite(bool favourite)
{
	if(save)
	{
		if (Client::Ref().FavouriteSave(save->id, favourite) == RequestOkay)
			save->Favourite = favourite;
		else if (favourite)
			throw PreviewModelException("Error, could not fav. the save, are you logged in?");
		else
			throw PreviewModelException("Error, could not unfav. the save, are you logged in?");
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
	this->tSaveID = saveID;
	this->tSaveDate = saveDate;

	if(save)
	{
		delete save;
		save = NULL;
	}
	if (saveData)
	{
		if (saveData->data)
			free(saveData->data);
		delete saveData;
		saveData = NULL;
	}
	if(saveComments)
	{
		for(int i = 0; i < saveComments->size(); i++)
			delete saveComments->at(i);
		saveComments->clear();
		delete saveComments;
		saveComments = NULL;
	}
	notifySaveChanged();
	notifySaveCommentsChanged();

	if (!updateSaveDataInfo)
		updateSaveDataInfo = new threadInfo(saveID, saveDate);
	if (updateSaveDataInfo->threadFinished)
	{
		updateSaveDataInfo->threadFinished = false;
		pthread_create(&updateSaveDataThread, 0, &PreviewModel::updateSaveDataT, updateSaveDataInfo);
	}

	if (!updateSaveInfoInfo)
		updateSaveInfoInfo = new threadInfo(saveID, saveDate);
	if(updateSaveInfoInfo->threadFinished)
	{
		updateSaveInfoInfo->threadFinished = false;
		pthread_create(&updateSaveInfoThread, 0, &PreviewModel::updateSaveInfoT, updateSaveInfoInfo);
	}

	if (!updateSaveCommentsInfo)
		updateSaveCommentsInfo = new threadInfo(saveID, commentsPageNumber);
	if (updateSaveCommentsInfo->threadFinished)
	{
		commentsLoaded = false;
		updateSaveCommentsInfo->threadFinished = false;
		pthread_create(&updateSaveCommentsThread, 0, &PreviewModel::updateSaveCommentsT, updateSaveCommentsInfo);
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

SaveInfo * PreviewModel::GetSave()
{
	return save;
}

int PreviewModel::GetCommentsPageNum()
{
	return commentsPageNumber;
}

int PreviewModel::GetCommentsPageCount()
{
	return max(1, (int)(ceil(commentsTotal/20.0f)));
}

bool PreviewModel::GetCommentsLoaded()
{
	return commentsLoaded;
}

void PreviewModel::UpdateComments(int pageNumber)
{
	commentsLoaded = false;
	if(saveComments)
	{
		for(int i = 0; i < saveComments->size(); i++)
			delete saveComments->at(i);
		saveComments->clear();
		delete saveComments;
		saveComments = NULL;
	}

	//resultCount = 0;
	commentsPageNumber = pageNumber;
	notifySaveCommentsChanged();
	notifyCommentsPageChanged();

	//Threading
	if (!updateSaveCommentsInfo)
		updateSaveCommentsInfo = new threadInfo(tSaveID, commentsPageNumber);
	if (updateSaveCommentsInfo->threadFinished)
	{
		commentsLoaded = false;
		updateSaveCommentsInfo->threadFinished = false;
		pthread_create(&updateSaveCommentsThread, 0, &PreviewModel::updateSaveCommentsT, updateSaveCommentsInfo);
	}
}

std::vector<SaveComment*> * PreviewModel::GetComments()
{
	return saveComments;
}

void PreviewModel::notifySaveChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifySaveChanged(this);
	}
}

void PreviewModel::notifyCommentBoxEnabledChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyCommentBoxEnabledChanged(this);
	}
}

void PreviewModel::notifyCommentsPageChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyCommentsPageChanged(this);
	}
}

void PreviewModel::notifySaveCommentsChanged()
{
	for(int i = 0; i < observers.size(); i++)
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

void PreviewModel::Update()
{
	if (updateSaveDataInfo && updateSaveDataInfo->threadFinished)
	{
		pthread_join(updateSaveDataThread, (void**)(&saveData));
		delete updateSaveDataInfo;
		updateSaveDataInfo = NULL;

		if (save && saveData)
		{
			commentsTotal = save->Comments;
			try
			{
				save->SetGameSave(new GameSave((char*)saveData->data, saveData->length));
			}
			catch(ParseException &e)
			{
				throw PreviewModelException("Save file corrupt or from newer version");
			}
			notifySaveChanged();
			notifyCommentsPageChanged();
		}
	}

	if (updateSaveInfoInfo && updateSaveInfoInfo->threadFinished)
	{
		if (save)
		{
			delete save;
			save = NULL;
		}
		pthread_join(updateSaveInfoThread, (void**)(&save));
		delete updateSaveInfoInfo;
		updateSaveInfoInfo = NULL;

		if (save && saveData)
		{
			commentsTotal = save->Comments;
			try
			{
				save->SetGameSave(new GameSave((char*)saveData->data, saveData->length));
			}
			catch(ParseException &e)
			{
				throw PreviewModelException("Save file corrupt or from newer version");
			}
			notifyCommentsPageChanged();
		}
		notifySaveChanged();

		if(!save)
			throw PreviewModelException("Unable to load save");
	}

	if (updateSaveCommentsInfo && updateSaveCommentsInfo->threadFinished)
	{
		if(saveComments)
		{
			for(int i = 0; i < saveComments->size(); i++)
				delete saveComments->at(i);
			saveComments->clear();
			delete saveComments;
			saveComments = NULL;
		}

		commentsLoaded = true;
		pthread_join(updateSaveCommentsThread, (void**)(&saveComments));
		delete updateSaveCommentsInfo;
		updateSaveCommentsInfo = NULL;
		notifySaveCommentsChanged();
	}
}

PreviewModel::~PreviewModel()
{
	if (updateSaveDataInfo)
		updateSaveDataInfo->previewExited = true;
	if (updateSaveInfoInfo)
		updateSaveInfoInfo->previewExited = true;
	if (updateSaveCommentsInfo)
		updateSaveCommentsInfo->previewExited = true;
	if(save)
		delete save;
	if(saveComments)
	{
		for(int i = 0; i < saveComments->size(); i++)
			delete saveComments->at(i);
		saveComments->clear();
		delete saveComments;
	}
	if (saveData)
	{
		if (saveData->data)
			free(saveData->data);
		delete saveData;
	}
}
