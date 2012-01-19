#include "SearchModel.h"
#include "Save.h"

SearchModel::SearchModel()
{
}

void SearchModel::UpdateSaveList()
{
	saveList.clear();
	notifySaveListChanged();
	for(int i = 0; i < 16; i++)
	{
		saveList.push_back(Save(2198, 2333, 315, "dima-gord", "Destroyable city 5 (wth metro)"));
	}
	notifySaveListChanged();
}

vector<Save> SearchModel::GetSaveList()
{
	return saveList;
}

void SearchModel::AddObserver(SearchView * observer)
{
	observers.push_back(observer);
	observer->NotifySaveListChanged(this);
}

void SearchModel::notifySaveListChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		SearchView* cObserver = observers[i];
		cObserver->NotifySaveListChanged(this);
	}
}
