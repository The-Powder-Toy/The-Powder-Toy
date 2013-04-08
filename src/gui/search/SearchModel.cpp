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
	updateTagListWorking(false),
	updateTagListFinished(false),
	saveListLoaded(false),
	currentPage(1),
	resultCount(0),
	showTags(true)
{
}

void SearchModel::SetShowTags(bool show)
{
	showTags = show;
}

bool SearchModel::GetShowTags()
{
	return showTags;	
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
	vector<SaveInfo*> * saveList = Client::Ref().SearchSaves((currentPage-1)*20, 20, lastQuery, currentSort=="new"?"date":"votes", category, thResultCount);

	updateSaveListFinished = true;
	return saveList;
}

void * SearchModel::updateTagListTHelper(void * obj)
{
	return ((SearchModel *)obj)->updateTagListT();
}

void * SearchModel::updateTagListT()
{
	int tagResultCount;
	std::vector<std::pair<std::string, int> > * tagList = Client::Ref().GetTags(0, 24, "", tagResultCount);

	updateTagListFinished = true;
	return tagList;
}

void SearchModel::UpdateSaveList(int pageNumber, std::string query)
{
	//Threading
	if(!updateSaveListWorking)
	{
		lastQuery = query;
		lastError = "";
		saveListLoaded = false;
		saveList.clear();
		//resultCount = 0;
		currentPage = pageNumber;

		if(pageNumber == 1 && !showOwn && !showFavourite && currentSort == "best" && query == "")
			SetShowTags(true);
		else
			SetShowTags(false);

		notifySaveListChanged();
		notifyTagListChanged();
		notifyPageChanged();
		selected.clear();
		notifySelectedChanged();

		if(GetShowTags() && !tagList.size() && !updateTagListWorking)
		{
			updateTagListFinished = false;
			updateTagListWorking = true;
			pthread_create(&updateTagListThread, 0, &SearchModel::updateTagListTHelper, this);
		}
		
		updateSaveListFinished = false;
		updateSaveListWorking = true;
		pthread_create(&updateSaveListThread, 0, &SearchModel::updateSaveListTHelper, this);
	}
}

void SearchModel::SetLoadedSave(SaveInfo * save)
{
	if(loadedSave != save && loadedSave)
		delete loadedSave;
	if(save)
	{
		loadedSave = new SaveInfo(*save);
	}
	else
	{
		loadedSave = NULL;
	}
}

SaveInfo * SearchModel::GetLoadedSave(){
	return loadedSave;
}

vector<SaveInfo*> SearchModel::GetSaveList()
{
	return saveList;
}

vector<pair<string, int> > SearchModel::GetTagList()
{
	return tagList;
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
			pthread_join(updateSaveListThread, (void**)&tempSaveList);

			if(tempSaveList)
			{
				saveList = *tempSaveList;
				delete tempSaveList;
			}

			if(!saveList.size())
			{
				lastError = Client::Ref().GetLastError();
			}
			
			resultCount = thResultCount;
			notifyPageChanged();
			notifySaveListChanged();
		}
	}
	if(updateTagListWorking)
	{
		if(updateTagListFinished)
		{
			updateTagListWorking = false;

			vector<pair<string, int> > * tempTagList;
			pthread_join(updateTagListThread, (void**)&tempTagList);

			if(tempTagList)
			{
				tagList = *tempTagList;
				delete tempTagList;
			}
			notifyTagListChanged();
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
	observer->NotifyTagListChanged(this);
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

void SearchModel::notifyTagListChanged()
{
	for(int i = 0; i < observers.size(); i++)
	{
		SearchView* cObserver = observers[i];
		cObserver->NotifyTagListChanged(this);
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
