/*
 * TagsModel.cpp
 *
 *  Created on: Mar 5, 2012
 *      Author: Simon
 */

#include "TagsModel.h"
#include "TagsView.h"

TagsModel::TagsModel():
	save(NULL)
{
	// TODO Auto-generated constructor stub

}

void TagsModel::SetSave(Save * save)
{
	this->save = save;
	notifyTagsChanged();
}

Save * TagsModel::GetSave()
{
	return save;
}

void TagsModel::AddObserver(TagsView * observer)
{
	observers.push_back(observer);
	observer->NotifyTagsChanged(this);
}

void TagsModel::notifyTagsChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		observers[i]->NotifyTagsChanged(this);
	}
}

TagsModel::~TagsModel() {
	// TODO Auto-generated destructor stub
}

