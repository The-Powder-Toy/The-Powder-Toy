#include "SearchModel.h"
#include "client/SaveInfo.h"

#include "client/Client.h"

SearchModel::SearchModel():
	currentSort("best"),
	showOwn(false),
	showFavourite(false),
	loadedSave(NULL),
	updateSaveListWorking(false),
	updateSaveListFinished(false),
	saveListLoaded(false),
	currentPage(1),
	resultCount(0)
{
}

void * SearchModel::updateSaveListTHelper(void * obj)
{
	return ((SearchModel *)obj)->updateSaveListT();
}

void * SearchModel::updateSaveListT()
{
	std::string category = "";
	if(showFavourite)
		category = "Favourites";
	if(showOwn && Client::Ref().GetAuthUser().ID)
		category = "by:"+Client::Ref().GetAuthUser().Username;
	vector<SaveInfo*> * tempSaveList = Client::Ref().SearchSaves((currentPage-1)*20, 20, lastQuery, currentSort=="new"?"date":"votes", category, resultCount);
	updateSaveListFinished = true;
	return tempSaveList;
}

void SearchModel::UpdateSaveList(int pageNumber, std::string query)
{
	lastQuery = query;
	lastError = "";
	saveListLoaded = false;
	saveList.clear();
	//resultCount = 0;
	currentPage = pageNumber;
	notifySaveListChanged();
	notifyPageChanged();
	selected.clear();
	notifySelectedChanged();

	//Threading
	if(!updateSaveListWorking)
	{
		updateSaveListFinished = false;
		updateSaveListWorking = true;
		pthread_create(&updateSaveListThread, 0, &SearchModel::updateSaveListTHelper, this);
	}
}

void SearchModel::SetLoadedSave(SaveInfo * save)
{
	loadedSave = save;
}

SaveInfo * SearchModel::GetLoadedSave(){
	return loadedSave;
}

vector<SaveInfo*> SearchModel::GetSaveList()
{
	return saveList;
}

void SearchModel::Update()
{
	if(updateSaveListWorking)
	{
		if(updateSaveListFinished)
		{
			updateSaveListWorking = false;
			lastError = "";
			saveListLoaded = true;
			vector<SaveInfo*> * tempSaveList;
			pthread_join(updateSaveListThread, (void**)(&tempSaveList));
			saveList = *tempSaveList;
			delete tempSaveList;
			if(!saveList.size())
			{
				lastError = Client::Ref().GetLastError();
			}
			//currentPage = pageNumber;
			notifyPageChanged();
			notifySaveListChanged();
		}
	}
}

void SearchModel::AddObserver(SearchView * observer)
{
	observers.push_back(observer);
	observer->NotifySaveListChanged(this);
	observer->NotifyPageChanged(this);
	observer->NotifySortChanged(this);
	observer->NotifyShowOwnChanged(this);
}

void SearchModel::SelectSave(int saveID)
{
	for(int i = 0; i < selected.size(); i++)
	{
		if(selected[i]==saveID)
		{
			return;
		}
	}
	selected.push_back(saveID);
	notifySelectedChanged();
}

void SearchModel::DeselectSave(int saveID)
{
	bool changed = false;
restart:
	for(int i = 0; i < selected.size(); i++)
	{
		if(selected[i]==saveID)
		{
			selected.erase(selected.begin()+i);
			changed = true;
			goto restart; //Just ensure all cases are removed.
		}
	}
	if(changed)
		notifySelectedChanged();
}

void SearchModel::notifySaveListChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		SearchView* cObserver = observers[i];
		cObserver->NotifySaveListChanged(this);
	}
}

void SearchModel::notifyPageChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		SearchView* cObserver = observers[i];
		cObserver->NotifyPageChanged(this);
	}
}

void SearchModel::notifySortChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		SearchView* cObserver = observers[i];
		cObserver->NotifySortChanged(this);
	}
}

void SearchModel::notifyShowOwnChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		SearchView* cObserver = observers[i];
		cObserver->NotifyShowOwnChanged(this);
	}
}

void SearchModel::notifyShowFavouriteChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		SearchView* cObserver = observers[i];
		cObserver->NotifyShowOwnChanged(this);
	}
}

void SearchModel::notifySelectedChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		SearchView* cObserver = observers[i];
		cObserver->NotifySelectedChanged(this);
	}
}

SearchModel::~SearchModel()
{
	if(loadedSave)
		delete loadedSave;
}
