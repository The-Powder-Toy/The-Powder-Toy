/*
 * StampsModel.cpp
 *
 *  Created on: Mar 29, 2012
 *      Author: Simon
 */

#include "StampsModel.h"
#include "StampsView.h"
#include "client/Client.h"
#include "StampsModelException.h"

StampsModel::StampsModel():
	stamp(NULL),
	currentPage(1)
{
	// TODO Auto-generated constructor stub
	stampIDs = Client::Ref().GetStamps();
}


std::vector<Save *> StampsModel::GetStampsList()
{
	return stampsList;
}

void StampsModel::AddObserver(StampsView * observer)
{
	observers.push_back(observer);
	observer->NotifyStampsListChanged(this);
	observer->NotifyPageChanged(this);
}

void StampsModel::notifyStampsListChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyStampsListChanged(this);
	}
}

void StampsModel::notifyPageChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyPageChanged(this);
	}
}

Save * StampsModel::GetStamp()
{
	return stamp;
}

void StampsModel::SetStamp(Save * newStamp)
{
	if(stamp)
		delete stamp;
	stamp = new Save(*newStamp);
}

void StampsModel::UpdateStampsList(int pageNumber)
{
	std::vector<Save*> tempStampsList = stampsList;
	stampsList.clear();
	currentPage = pageNumber;
	notifyPageChanged();
	/*notifyStampsListChanged();
	for(int i = 0; i < tempStampsList.size(); i++)
	{
		delete tempStampsList[i];
	}*/

	int stampsEnd = pageNumber*20;

	for(int i = stampsEnd-20; i<stampIDs.size() && i<stampsEnd; i++)
	{
		Save * tempSave = Client::Ref().GetStamp(stampIDs[i]);
		stampsList.push_back(tempSave);
	}
	notifyStampsListChanged();
}

StampsModel::~StampsModel() {
	if(stamp)
		delete stamp;
}

