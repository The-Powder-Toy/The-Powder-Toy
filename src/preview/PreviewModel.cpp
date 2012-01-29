/*
 * PreviewModel.cpp
 *
 *  Created on: Jan 21, 2012
 *      Author: Simon
 */

#include "PreviewModel.h"
#include "client/Client.h"

PreviewModel::PreviewModel():
	save(NULL),
	savePreview(NULL),
	doOpen(false),
	updateSavePreviewWorking(false),
	updateSavePreviewFinished(false),
	updateSaveInfoWorking(false),
	updateSaveInfoFinished(false)
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

void PreviewModel::UpdateSave(int saveID, int saveDate)
{
	this->tSaveID = saveID;
	this->tSaveDate = saveDate;

	save = NULL;
	savePreview = NULL;
	notifyPreviewChanged();
	notifySaveChanged();

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
			updateSavePreviewWorking = false;
			pthread_join(updateSavePreviewThread, (void**)(&savePreview));
			notifyPreviewChanged();
		}
	}

	if(updateSaveInfoWorking)
	{
		if(updateSaveInfoFinished)
		{
			updateSaveInfoWorking = false;
			pthread_join(updateSaveInfoThread, (void**)(&save));
			notifySaveChanged();
		}
	}
}

PreviewModel::~PreviewModel() {
	if(save)
		delete save;
	if(savePreview)
		delete savePreview;
}

