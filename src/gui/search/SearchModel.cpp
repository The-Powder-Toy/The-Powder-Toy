#include "SearchModel.h"
#include "SearchView.h"
#include "Format.h"
#include "client/SaveInfo.h"
#include "client/GameSave.h"
#include "client/Client.h"
#include "client/http/SearchSavesRequest.h"
#include "client/http/SearchTagsRequest.h"
#include <algorithm>
#include <thread>
#include <cmath>

SearchModel::SearchModel():
	currentPeriod(http::allSaves),
	currentSort(http::sortByVotes),
	currentPage(1),
	resultCount(0),
	includesFp(false),
	showOwn(false),
	showFavourite(false),
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

void SearchModel::BeginSearchSaves(int start, int count, String query, http::Period period, http::Sort sort, http::Category category)
{
	lastError = "";
	resultCount = 0;
	searchSaves = std::make_unique<http::SearchSavesRequest>(start, count, query.ToUtf8(), period, sort, category);
	searchSaves->Start();
	includesFp = searchSaves->GetIncludesFp();
}

std::vector<std::unique_ptr<SaveInfo>> SearchModel::EndSearchSaves()
{
	std::vector<std::unique_ptr<SaveInfo>> saveArray;
	try
	{
		std::tie(resultCount, saveArray) = searchSaves->Finish();
	}
	catch (const http::RequestError &ex)
	{
		lastError = ByteString(ex.what()).FromUtf8();
	}
	searchSaves.reset();
	return saveArray;
}

void SearchModel::BeginGetTags(int start, int count, String query)
{
	lastError = "";
	getTags = std::make_unique<http::SearchTagsRequest>(start, count, query.ToUtf8());
	getTags->Start();
}

std::vector<std::pair<ByteString, int>> SearchModel::EndGetTags()
{
	std::vector<std::pair<ByteString, int>> tagArray;
	try
	{
		tagArray = getTags->Finish();
	}
	catch (const http::RequestError &ex)
	{
		lastError = ByteString(ex.what()).FromUtf8();
	}
	getTags.reset();
	return tagArray;
}

bool SearchModel::UpdateSaveList(int pageNumber, String query)
{
	//Threading
	if (!searchSaves)
	{
		lastQuery = query;
		lastError = "";
		saveListLoaded = false;
		saveList.clear();
		//resultCount = 0;
		currentPage = pageNumber;

		auto category = http::categoryNone;
		if (showFavourite)
		{
			category = http::categoryFavourites;
		}
		if (showOwn && Client::Ref().GetAuthUser())
		{
			category = http::categoryMyOwn;
		}
		BeginSearchSaves((currentPage-1)*20, 20, lastQuery, currentPeriod, currentSort, category);

		SetShowTags(includesFp && pageNumber == 1);

		notifySaveListChanged();
		notifyTagListChanged();
		notifyPageChanged();
		selected.clear();
		notifySelectedChanged();

		if (GetShowTags() && !tagList.size() && !getTags)
		{
			BeginGetTags(0, 24, "");
		}

		return true;
	}
	return false;
}

void SearchModel::SetLoadedSave(std::unique_ptr<SaveInfo> save)
{
	loadedSave = std::move(save);
}

const SaveInfo *SearchModel::GetLoadedSave() const
{
	return loadedSave.get();
}

std::unique_ptr<SaveInfo> SearchModel::TakeLoadedSave()
{
	return std::move(loadedSave);
}

std::vector<SaveInfo *> SearchModel::GetSaveList() // non-owning
{
	std::vector<SaveInfo *> nonOwningSaveList;
	std::transform(saveList.begin(), saveList.end(), std::back_inserter(nonOwningSaveList), [](auto &ptr) {
		return ptr.get();
	});
	return nonOwningSaveList;
}

std::vector<std::pair<ByteString, int> > SearchModel::GetTagList()
{
	return tagList;
}

void SearchModel::Update()
{
	if (searchSaves && searchSaves->CheckDone())
	{
		saveListLoaded = true;
		lastError = "";
		saveList = EndSearchSaves();
		notifyPageChanged();
		notifySaveListChanged();
	}
	if (getTags && getTags->CheckDone())
	{
		lastError = "";
		tagList = EndGetTags();
		notifyTagListChanged();
	}
}

void SearchModel::AddObserver(SearchView * observer)
{
	observers.push_back(observer);
	observer->NotifySaveListChanged(this);
	observer->NotifyPageChanged(this);
	observer->NotifyPeriodChanged(this);
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
	if (std::erase_if(selected, [saveID](auto &item) {
		return item == saveID;
	}))
	{
		notifySelectedChanged();
	}
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

void SearchModel::notifyPeriodChanged()
{
	for (size_t i = 0; i < observers.size(); i++)
	{
		SearchView* cObserver = observers[i];
		cObserver->NotifyPeriodChanged(this);
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

int SearchModel::GetPageCount()
{
	return std::max(1, (int)(ceil(resultCount/20.0f)) + (includesFp ? 1 : 0)); //add one for front page (front page saves are repeated twice)
}
