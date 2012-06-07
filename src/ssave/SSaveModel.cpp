/*
 * SSaveModel.cpp
 *
 *  Created on: Jan 29, 2012
 *      Author: Simon
 */

#include "SSaveModel.h"
#include "client/Client.h"

SSaveModel::SSaveModel():
	save(NULL),
	saveUploaded(false)
{
	// TODO Auto-generated constructor stub

}

void SSaveModel::notifySaveChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifySaveChanged(this);
	}
}

void SSaveModel::notifySaveUploadChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifySaveUploadChanged(this);
	}
}

void SSaveModel::UploadSave(std::string saveName, std::string saveDescription, bool publish)
{
	save->name = saveName;
	save->Description = saveDescription;
	save->Published = publish;
	saveUploaded = false;
	notifySaveUploadChanged();

	if(Client::Ref().UploadSave(save) == RequestOkay)
	{
		saveUploaded = true;
	}
	else
	{
		saveUploaded = false;
	}
	notifySaveUploadChanged();
}

void SSaveModel::SetSave(SaveInfo * save)
{
	this->save = save;
	notifySaveChanged();
}

SaveInfo * SSaveModel::GetSave()
{
	return this->save;
}

bool SSaveModel::GetSaveUploaded()
{
	return saveUploaded;
}

void SSaveModel::AddObserver(SSaveView * observer)
{
	observers.push_back(observer);
	observer->NotifySaveChanged(this);
	observer->NotifySaveUploadChanged(this);
}

void SSaveModel::Update()
{

}

SSaveModel::~SSaveModel() {
	delete save;
}

