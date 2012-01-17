#include "SearchModel.h"
#include "Save.h"

SearchModel::SearchModel()
{
}

void SearchModel::UpdateSaveList()
{
	saveList.clear();
	notifySaveListChanged();
	saveList.push_back(Save(1, 45, 5, "Simon", "Post logic gates"));
	notifySaveListChanged();
}

void SearchModel::notifySaveListChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		SearchView* cObserver = observers[i];
		cObserver->NotifySaveListChanged(this);
	}
}
