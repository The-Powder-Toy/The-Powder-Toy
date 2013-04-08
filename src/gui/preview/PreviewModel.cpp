#include <cmath>
#include "PreviewModel.h"
#include "client/Client.h"
#include "client/GameSave.h";
#include "PreviewModelException.h"

PreviewModel::PreviewModel():
	save(NULL),
	saveComments(NULL),
	doOpen(false),
	updateSaveDataWorking(false),
	updateSaveDataFinished(false),
	updateSaveInfoWorking(false),
	updateSaveInfoFinished(false),
	updateSaveCommentsWorking(false),
	updateSaveCommentsFinished(false),
	commentsTotal(0),
	commentsPageNumber(1),
	commentBoxEnabled(false)
{

}

void * PreviewModel::updateSaveInfoTHelper(void * obj)
{
	return ((PreviewModel*)obj)->updateSaveInfoT();
}

void * PreviewModel::updateSaveDataTHelper(void * obj)
{
	return ((PreviewModel*)obj)->updateSaveDataT();
}

void * PreviewModel::updateSaveCommentsTHelper(void * obj)
{
	return ((PreviewModel*)obj)->updateSaveCommentsT();
}

void PreviewModel::updateSaveInfoTDelete(void * arg)
{
	delete arg;
}
void PreviewModel::updateSaveDataTDelete(void * arg)
{
	free(arg);
}
void PreviewModel::updateSaveCommentsTDelete(void * arg)
{
	for(int i = 0; i < ((std::vector<SaveComment*> *)arg)->size(); i++)
		delete ((std::vector<SaveComment*> *)arg)->at(i);
	((std::vector<SaveComment*> *)arg)->clear();
	delete arg;
}

void * PreviewModel::updateSaveInfoT()
{
	SaveInfo * tempSave;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
	tempSave = Client::Ref().GetSave(tSaveID, tSaveDate);
	pthread_cleanup_push(&updateSaveInfoTDelete,tempSave);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	pthread_testcancel();
	updateSaveInfoFinished = true;
	pthread_cleanup_pop(0);
	return tempSave;
}

void * PreviewModel::updateSaveDataT()
{
	int tempDataSize;
	unsigned char * tempData;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
	tempData = Client::Ref().GetSaveData(tSaveID, tSaveDate, tempDataSize);
	pthread_cleanup_push(&updateSaveDataTDelete,tempData);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	pthread_testcancel();
	saveDataBuffer.clear();
	if (tempData)
		saveDataBuffer.insert(saveDataBuffer.begin(), tempData, tempData+tempDataSize);
	updateSaveDataFinished = true;
	pthread_cleanup_pop(1);
	return NULL;
}

void * PreviewModel::updateSaveCommentsT()
{
	std::vector<SaveComment*> * tempComments;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
	tempComments = Client::Ref().GetComments(tSaveID, (commentsPageNumber-1)*20, 20);
	pthread_cleanup_push(&updateSaveCommentsTDelete,tempComments);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	pthread_testcancel();
	updateSaveCommentsFinished = true;
	pthread_cleanup_pop(0);
	return tempComments;
}

void PreviewModel::SetFavourite(bool favourite)
{
	if(save)
	{
		Client::Ref().FavouriteSave(save->id, favourite);
		save->Favourite = favourite;
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
	saveDataBuffer.clear();
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

	if(!updateSaveDataWorking)
	{
		updateSaveDataWorking = true;
		updateSaveDataFinished = false;
		pthread_create(&updateSaveDataThread, 0, &PreviewModel::updateSaveDataTHelper, this);
	}

	if(!updateSaveInfoWorking)
	{
		updateSaveInfoWorking = true;
		updateSaveInfoFinished = false;
		pthread_create(&updateSaveInfoThread, 0, &PreviewModel::updateSaveInfoTHelper, this);
	}

	if(!updateSaveCommentsWorking)
	{
		commentsLoaded = false;
		updateSaveCommentsWorking = true;
		updateSaveCommentsFinished = false;
		pthread_create(&updateSaveCommentsThread, 0, &PreviewModel::updateSaveCommentsTHelper, this);
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
	if(!updateSaveCommentsWorking)
	{
		updateSaveCommentsFinished = false;
		updateSaveCommentsWorking = true;
		pthread_create(&updateSaveCommentsThread, 0, &PreviewModel::updateSaveCommentsTHelper, this);
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

void PreviewModel::AddObserver(PreviewView * observer) {
	observers.push_back(observer);
	observer->NotifySaveChanged(this);
	observer->NotifyCommentsChanged(this);
	observer->NotifyCommentsPageChanged(this);
	observer->NotifyCommentBoxEnabledChanged(this);
}

void PreviewModel::Update()
{
	if(updateSaveDataWorking)
	{
		if(updateSaveDataFinished)
		{
			updateSaveDataWorking = false;
			pthread_join(updateSaveDataThread, NULL);

			if(updateSaveInfoFinished && save)
			{
				commentsTotal = save->Comments;
				try
				{
					save->SetGameSave(new GameSave(&saveDataBuffer[0], saveDataBuffer.size()));
				}
				catch(ParseException &e)
				{
					throw PreviewModelException("Save file corrupt or from newer version");
				}
				notifySaveChanged();
				notifyCommentsPageChanged();
			}
		}
	}

	if(updateSaveInfoWorking)
	{
		if(updateSaveInfoFinished)
		{
			if(save)
			{
				delete save;
				save = NULL;
			}
			updateSaveInfoWorking = false;
			pthread_join(updateSaveInfoThread, (void**)(&save));
			if(updateSaveDataFinished && save)
			{
				commentsTotal = save->Comments;
				try
				{
					save->SetGameSave(new GameSave(&saveDataBuffer[0], saveDataBuffer.size()));
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
	}

	if(updateSaveCommentsWorking)
	{
		if(updateSaveCommentsFinished)
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
			updateSaveCommentsWorking = false;
			pthread_join(updateSaveCommentsThread, (void**)(&saveComments));
			notifySaveCommentsChanged();
		}
	}
}

PreviewModel::~PreviewModel() {
	pthread_cancel(updateSaveDataThread);
	pthread_cancel(updateSaveInfoThread);
	pthread_cancel(updateSaveCommentsThread);
	if(save)
		delete save;
	if(saveComments)
	{
		for(int i = 0; i < saveComments->size(); i++)
			delete saveComments->at(i);
		saveComments->clear();
		delete saveComments;
	}
	saveDataBuffer.clear();
}

