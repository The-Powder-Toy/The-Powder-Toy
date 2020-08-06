#include "SearchModel.h"

#include "SearchView.h"

#include "client/SaveInfo.h"
#include "client/Client.h"

#include <thread>
#include <cmath>

#include "common/tpt-minmax.h"

SearchModel::SearchModel():
	loadedSave(NULL),
	currentSort("best"),
	currentPage(1),
	resultCount(0),
	showOwn(false),
	showFavourite(false),
	showTags(true),
	saveListLoaded(false),
	updateSaveListWorking(false),
	updateSaveListFinished(false),
	updateSaveListResult(nullptr),
	updateTagListWorking(false),
	updateTagListFinished(false),
	updateTagListResult(nullptr)
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

void SearchModel::updateSaveListT()
{
	ByteString category = "";
	if(showFavourite)
		category = "Favourites";
	if(showOwn && Client::Ref().GetAuthUser().UserID)
		category = "by:"+Client::Ref().GetAuthUser().Username;
	std::vector<SaveInfo*> * saveList = Client::Ref().SearchSaves((currentPage-1)*20, 20, lastQuery, currentSort=="new"?"date":"votes", category, thResultCount);

	updateSaveListResult = saveList;
	updateSaveListFinished = true;
}

void SearchModel::updateTagListT()
{
	int tagResultCount;
	std::vector<std::pair<ByteString, int> > * tagList = Client::Ref().GetTags(0, 24, "", tagResultCount);

	updateTagListResult = tagList;
	updateTagListFinished = true;
}

bool SearchModel::UpdateSaveList(int pageNumber, String query)
{
	//Threading
	if (!updateSaveListWorking)
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
			std::thread([this]() { updateTagListT(); }).detach();
		}

		updateSaveListFinished = false;
		updateSaveListWorking = true;
		std::thread([this]() { updateSaveListT(); }).detach();
		return true;
	}
	return false;
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

std::vector<SaveInfo*> SearchModel::GetSaveList()
{
	return saveList;
}

std::vector<std::pair<ByteString, int> > SearchModel::GetTagList()
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

			std::vector<SaveInfo *> *tempSaveList = updateSaveListResult;
			updateSaveListResult = nullptr;

			if(tempSaveList)
			{
				saveList = *tempSaveList;
				delete tempSaveList;
			}

			if(!saveList.size())
			{
				lastError = Client::Ref().GetLastError();
				if (lastError == "Unspecified Error")
					lastError = "";
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

			std::vector<std::pair<ByteString, int>> *tempTagList = updateTagListResult;
			updateTagListResult = nullptr;

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
	for (size_t i = 0; i < selected.size(); i++)
	{
		if (selected[i] == saveID)
		{
			return;
		}
	}
	selected.push_back(saveID);
	notifySelectedChanged();
}

void SearchModel::SelectAllSaves()
{
	if (selected.size() == saveList.size())
	{
		for (auto &save : saveList)
		{
			DeselectSave(save->id);
		}
	}
	else
	{
		for (auto &save : saveList)
		{
			SelectSave(save->id);
		}
	}
}

void SearchModel::DeselectSave(int saveID)
{
	bool changed = false;
restart:
	for (size_t i = 0; i < selected.size(); i++)
	{
		if (selected[i] == saveID)
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
	for (size_t i = 0; i < observers.size(); i++)
	{
		SearchView* cObserver = observers[i];
		cObserver->NotifySaveListChanged(this);
	}
}

void SearchModel::notifyTagListChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		SearchView* cObserver = observers[i];
		cObserver->NotifyTagListChanged(this);
	}
}

void SearchModel::notifyPageChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		SearchView* cObserver = observers[i];
		cObserver->NotifyPageChanged(this);
	}
}

void SearchModel::notifySortChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		SearchView* cObserver = observers[i];
		cObserver->NotifySortChanged(this);
	}
}

void SearchModel::notifyShowOwnChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		SearchView* cObserver = observers[i];
		cObserver->NotifyShowOwnChanged(this);
	}
}

void SearchModel::notifyShowFavouriteChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		SearchView* cObserver = observers[i];
		cObserver->NotifyShowOwnChanged(this);
	}
}

void SearchModel::notifySelectedChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		SearchView* cObserver = observers[i];
		cObserver->NotifySelectedChanged(this);
	}
}

SearchModel::~SearchModel()
{
	delete loadedSave;
}

int SearchModel::GetPageCount()
{
	if (!showOwn && !showFavourite && currentSort == "best" && lastQuery == "")
		return std::max(1, (int)(ceil(resultCount/20.0f))+1); //add one for front page (front page saves are repeated twice)
	else
		return std::max(1, (int)(ceil(resultCount/20.0f)));
}
