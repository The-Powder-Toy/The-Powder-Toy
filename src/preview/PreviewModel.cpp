/*
 * PreviewModel.cpp
 *
 *  Created on: Jan 21, 2012
 *      Author: Simon
 */

#include "PreviewModel.h"
#include "client/Client.h"
#include "PreviewModelException.h"

PreviewModel::PreviewModel():
	save(NULL),
	savePreview(NULL),
	saveComments(NULL),
	doOpen(false),
	updateSavePreviewWorking(false),
	updateSavePreviewFinished(false),
	updateSaveInfoWorking(false),
	updateSaveInfoFinished(false),
	updateSaveCommentsWorking(false),
	updateSaveCommentsFinished(false)
{
	// TODO Auto-generated constructor stub

}

void * PreviewModel::updateSaveInfoTHelper(void * obj)
{
	return ((PreviewModel*)obj)->updateSaveInfoT();
}

void * PreviewModel::updateSavePreviewTHelper(void * obj)
{
	return ((PreviewModel*)obj)->updateSavePreviewT();
}

void * PreviewModel::updateSaveCommentsTHelper(void * obj)
{
	return ((PreviewModel*)obj)->updateSaveCommentsT();
}

void * PreviewModel::updateSaveInfoT()
{
	Save * tempSave = Client::Ref().GetSave(tSaveID, tSaveDate);
	updateSaveInfoFinished = true;
	return tempSave;
}

void * PreviewModel::updateSavePreviewT()
{
	Thumbnail * tempThumb = Client::Ref().GetPreview(tSaveID, tSaveDate);
	updateSavePreviewFinished = true;
	return tempThumb;
}

void * PreviewModel::updateSaveCommentsT()
{
	std::vector<SaveComment*> * tempComments = Client::Ref().GetComments(tSaveID, 0, 10);
	updateSaveCommentsFinished = true;
	return tempComments;
}

void PreviewModel::SetFavourite(bool favourite)
{
	//if(save)
	{
		Client::Ref().FavouriteSave(save->id, favourite);
		save->Favourite = favourite;
		notifySaveChanged();
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
	if(savePreview)
	{
		delete savePreview;
		savePreview = NULL;
	}
	if(saveComments)
	{
		for(int i = 0; i < saveComments->size(); i++)
			delete saveComments->at(i);
		delete saveComments;
		saveComments = NULL;
	}
	notifyPreviewChanged();
	notifySaveChanged();
	notifySaveCommentsChanged();

	if(!updateSavePreviewWorking)
	{
		updateSavePreviewWorking = true;
		updateSavePreviewFinished = false;
		pthread_create(&updateSavePreviewThread, 0, &PreviewModel::updateSavePreviewTHelper, this);
	}

	if(!updateSaveInfoWorking)
	{
		updateSaveInfoWorking = true;
		updateSaveInfoFinished = false;
		pthread_create(&updateSaveInfoThread, 0, &PreviewModel::updateSaveInfoTHelper, this);
	}

	if(!updateSaveCommentsWorking)
	{
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

Thumbnail * PreviewModel::GetPreview()
{
	return savePreview;
}

Save * PreviewModel::GetSave()
{
	return save;
}

std::vector<SaveComment*> * PreviewModel::GetComments()
{
	return saveComments;
}

void PreviewModel::notifyPreviewChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyPreviewChanged(this);
	}
}

void PreviewModel::notifySaveChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifySaveChanged(this);
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
	observer->NotifyPreviewChanged(this);
	observer->NotifySaveChanged(this);
}

void PreviewModel::Update()
{
	if(updateSavePreviewWorking)
	{
		if(updateSavePreviewFinished)
		{
			if(savePreview)
			{
				delete savePreview;
				savePreview = NULL;
			}
			updateSavePreviewWorking = false;
			pthread_join(updateSavePreviewThread, (void**)(&savePreview));
			notifyPreviewChanged();
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
				delete saveComments;
				saveComments = NULL;
			}
			updateSaveCommentsWorking = false;
			pthread_join(updateSaveCommentsThread, (void**)(&saveComments));
			notifySaveCommentsChanged();
		}
	}
}

PreviewModel::~PreviewModel() {
	if(save)
		delete save;
	if(savePreview)
		delete savePreview;
}

