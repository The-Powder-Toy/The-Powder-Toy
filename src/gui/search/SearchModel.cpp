#include "SearchModel.h"
#include "SearchView.h"
#include "Format.h"
#include "client/SaveInfo.h"
#include "client/GameSave.h"
#include "client/Client.h"
#include "common/tpt-minmax.h"
#include <thread>
#include <cmath>

SearchModel::SearchModel():
	currentSort("best"),
	currentPage(1),
	resultCount(0),
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

void SearchModel::BeginSearchSaves(int start, int count, String query, ByteString sort, ByteString category)
{
	lastError = "";
	resultCount = 0;
	ByteStringBuilder urlStream;
	ByteString data;
	urlStream << SCHEME << SERVER << "/Browse.json?Start=" << start << "&Count=" << count;
	if(query.length() || sort.length())
	{
		urlStream << "&Search_Query=";
		if(query.length())
			urlStream << format::URLEncode(query.ToUtf8());
		if(sort == "date")
		{
			if(query.length())
				urlStream << format::URLEncode(" ");
			urlStream << format::URLEncode("sort:") << format::URLEncode(sort);
		}
	}
	if(category.length())
	{
		urlStream << "&Category=" << format::URLEncode(category);
	}
	searchSaves = std::make_unique<http::Request>(urlStream.Build());
	auto authUser = Client::Ref().GetAuthUser();
	if (authUser.UserID)
	{
		searchSaves->AuthHeaders(ByteString::Build(Client::Ref().GetAuthUser().UserID), Client::Ref().GetAuthUser().SessionID);
	}
	searchSaves->Start();
}

std::vector<std::unique_ptr<SaveInfo>> SearchModel::EndSearchSaves()
{
	std::vector<std::unique_ptr<SaveInfo>> saveArray;
	auto [ dataStatus, data ] = searchSaves->Finish();
	searchSaves.reset();
	auto &client = Client::Ref();
	client.ParseServerReturn(data, dataStatus, true);
	if (dataStatus == 200 && data.size())
	{
		try
		{
			std::istringstream dataStream(data);
			Json::Value objDocument;
			dataStream >> objDocument;

			resultCount = objDocument["Count"].asInt();
			Json::Value savesArray = objDocument["Saves"];
			for (Json::UInt j = 0; j < savesArray.size(); j++)
			{
				int tempID = savesArray[j]["ID"].asInt();
				int tempCreatedDate = savesArray[j]["Created"].asInt();
				int tempUpdatedDate = savesArray[j]["Updated"].asInt();
				int tempScoreUp = savesArray[j]["ScoreUp"].asInt();
				int tempScoreDown = savesArray[j]["ScoreDown"].asInt();
				ByteString tempUsername = savesArray[j]["Username"].asString();
				String tempName = ByteString(savesArray[j]["Name"].asString()).FromUtf8();
				int tempVersion = savesArray[j]["Version"].asInt();
				bool tempPublished = savesArray[j]["Published"].asBool();
				auto tempSaveInfo = std::make_unique<SaveInfo>(tempID, tempCreatedDate, tempUpdatedDate, tempScoreUp, tempScoreDown, tempUsername, tempName);
				tempSaveInfo->Version = tempVersion;
				tempSaveInfo->SetPublished(tempPublished);
				saveArray.push_back(std::move(tempSaveInfo));
			}
		}
		catch (std::exception &e)
		{
			lastError = "Could not read response: " + ByteString(e.what()).FromUtf8();
		}
	}
	else
	{
		lastError = client.GetLastError();
	}
	return saveArray;
}

void SearchModel::BeginGetTags(int start, int count, String query)
{
	lastError = "";
	ByteStringBuilder urlStream;
	urlStream << SCHEME << SERVER << "/Browse/Tags.json?Start=" << start << "&Count=" << count;
	if(query.length())
	{
		urlStream << "&Search_Query=";
		if(query.length())
			urlStream << format::URLEncode(query.ToUtf8());
	}
	getTags = std::make_unique<http::Request>(urlStream.Build());
	getTags->Start();
}

std::vector<std::pair<ByteString, int>> SearchModel::EndGetTags()
{
	std::vector<std::pair<ByteString, int>> tagArray;
	auto [ dataStatus, data ] = getTags->Finish();
	getTags.reset();
	if(dataStatus == 200 && data.size())
	{
		try
		{
			std::istringstream dataStream(data);
			Json::Value objDocument;
			dataStream >> objDocument;

			Json::Value tagsArray = objDocument["Tags"];
			for (Json::UInt j = 0; j < tagsArray.size(); j++)
			{
				int tagCount = tagsArray[j]["Count"].asInt();
				ByteString tag = tagsArray[j]["Tag"].asString();
				tagArray.push_back(std::pair<ByteString, int>(tag, tagCount));
			}
		}
		catch (std::exception & e)
		{
			lastError = "Could not read response: " + ByteString(e.what()).FromUtf8();
		}
	}
	else
	{
		lastError = http::StatusText(dataStatus);
	}
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

		if(pageNumber == 1 && !showOwn && !showFavourite && currentSort == "best" && query == "")
			SetShowTags(true);
		else
			SetShowTags(false);

		notifySaveListChanged();
		notifyTagListChanged();
		notifyPageChanged();
		selected.clear();
		notifySelectedChanged();

		if (GetShowTags() && !tagList.size() && !getTags)
		{
			BeginGetTags(0, 24, "");
		}

		ByteString category = "";
		if(showFavourite)
			category = "Favourites";
		if(showOwn && Client::Ref().GetAuthUser().UserID)
			category = "by:"+Client::Ref().GetAuthUser().Username;
		BeginSearchSaves((currentPage-1)*20, 20, lastQuery, currentSort=="new"?"date":"votes", category);
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

int SearchModel::GetPageCount()
{
	if (!showOwn && !showFavourite && currentSort == "best" && lastQuery == "")
		return std::max(1, (int)(ceil(resultCount/20.0f))+1); //add one for front page (front page saves are repeated twice)
	else
		return std::max(1, (int)(ceil(resultCount/20.0f)));
}
