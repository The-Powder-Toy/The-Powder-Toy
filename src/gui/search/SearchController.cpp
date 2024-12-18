#include "SearchController.h"

#include "Controller.h"
#include "SearchModel.h"
#include "SearchView.h"

#include "client/Client.h"
#include "client/SaveInfo.h"
#include "client/GameSave.h"
#include "client/http/DeleteSaveRequest.h"
#include "client/http/PublishSaveRequest.h"
#include "client/http/UnpublishSaveRequest.h"
#include "client/http/FavouriteSaveRequest.h"
#include "client/http/SearchSavesRequest.h"
#include "client/http/SearchTagsRequest.h"
#include "common/platform/Platform.h"
#include "graphics/Graphics.h"
#include "graphics/VideoBuffer.h"
#include "tasks/Task.h"
#include "tasks/TaskWindow.h"

#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/preview/PreviewController.h"
#include "gui/preview/PreviewView.h"
#include "SimulationConfig.h"
#include <algorithm>

SearchController::SearchController(std::function<void ()> onDone_):
	activePreview(nullptr),
	nextQueryTime(0.0f),
	nextQueryDone(true),
	instantOpen(false),
	doRefresh(false),
	HasExited(false)
{
	searchModel = new SearchModel();
	searchView = new SearchView();
	searchModel->AddObserver(searchView);
	searchView->AttachController(this);

	searchModel->UpdateSaveList(1, "");

	onDone = onDone_;
}

const SaveInfo *SearchController::GetLoadedSave() const
{
	return searchModel->GetLoadedSave();
}

std::unique_ptr<SaveInfo> SearchController::TakeLoadedSave()
{
	return searchModel->TakeLoadedSave();
}

void SearchController::Update()
{
	if (doRefresh)
	{
		if (searchModel->UpdateSaveList(searchModel->GetPageNum(), searchModel->GetLastQuery()))
		{
			nextQueryDone = true;
			doRefresh = false;
		}
	}
	else if (!nextQueryDone && nextQueryTime < Platform::GetTime())
	{
		if (searchModel->UpdateSaveList(1, nextQuery))
			nextQueryDone = true;
	}
	searchModel->Update();
	if(activePreview && activePreview->HasExited)
	{
		delete activePreview;
		activePreview = nullptr;
		if(searchModel->GetLoadedSave())
		{
			Exit();
		}
	}
}

void SearchController::Exit()
{
	InstantOpen(false);
	searchView->CloseActiveWindow();
	if (onDone)
		onDone();
	//HasExited = true;
}

SearchController::~SearchController()
{
	delete activePreview;
	delete searchModel;
	searchView->CloseActiveWindow();
	delete searchView;
}

void SearchController::DoSearch(String query, bool now)
{
	nextQuery = query;
	if (!now)
	{
		nextQueryTime = Platform::GetTime()+600;
		nextQueryDone = false;
	}
	else
	{
		nextQueryDone = searchModel->UpdateSaveList(1, nextQuery);
	}
}

void SearchController::DoSearch2(String query)
{
	// calls SearchView function to set textbox text, then calls DoSearch
	searchView->Search(query);
}

void SearchController::Refresh()
{
	doRefresh = true;
}

void SearchController::SetPage(int page)
{
	if (page != searchModel->GetPageNum() && page > 0 && page <= searchModel->GetPageCount())
		searchModel->UpdateSaveList(page, searchModel->GetLastQuery());
}

void SearchController::SetPageRelative(int offset)
{
	int page = std::min(std::max(searchModel->GetPageNum() + offset, 1), searchModel->GetPageCount());
	if (page != searchModel->GetPageNum())
		searchModel->UpdateSaveList(page, searchModel->GetLastQuery());
}

void SearchController::ChangePeriod(int period)
{
	switch(period)
	{
		case 0:
			searchModel->SetPeriod(http::allSaves);
			break;
		case 1:
			searchModel->SetPeriod(http::todaySaves);
			break;
		case 2:
			searchModel->SetPeriod(http::weekSaves);
			break;
		case 3:
			searchModel->SetPeriod(http::monthSaves);
			break;
		case 4:
			searchModel->SetPeriod(http::yearSaves);
			break;
		default:
			searchModel->SetPeriod(http::allSaves);
	}

	searchModel->UpdateSaveList(1, searchModel->GetLastQuery());
}

void SearchController::ChangeSort()
{
	if(searchModel->GetSort() == http::sortByDate)
	{
		searchModel->SetSort(http::sortByVotes);
	}
	else
	{
		searchModel->SetSort(http::sortByDate);
	}
	searchModel->UpdateSaveList(1, searchModel->GetLastQuery());
}

void SearchController::ShowOwn(bool show)
{
	if(Client::Ref().GetAuthUser().UserID)
	{
		searchModel->SetShowFavourite(false);
		searchModel->SetShowOwn(show);
	}
	else
		searchModel->SetShowOwn(false);
	searchModel->UpdateSaveList(1, searchModel->GetLastQuery());
}

void SearchController::ShowFavourite(bool show)
{
	if(Client::Ref().GetAuthUser().UserID)
	{
		searchModel->SetShowOwn(false);
		searchModel->SetShowFavourite(show);
	}
	else
		searchModel->SetShowFavourite(false);
	searchModel->UpdateSaveList(1, searchModel->GetLastQuery());
}

void SearchController::Selected(int saveID, bool selected)
{
	if(!Client::Ref().GetAuthUser().UserID)
		return;

	if(selected)
		searchModel->SelectSave(saveID);
	else
		searchModel->DeselectSave(saveID);
}

void SearchController::SelectAllSaves() 
{
	if (!Client::Ref().GetAuthUser().UserID)
		return;
	if (searchModel->GetShowOwn() || 
		Client::Ref().GetAuthUser().UserElevation == User::ElevationMod || 
		Client::Ref().GetAuthUser().UserElevation == User::ElevationAdmin)
		searchModel->SelectAllSaves();

}

void SearchController::InstantOpen(bool instant)
{
	instantOpen = instant;
}

void SearchController::OpenSaveDone()
{
	if (activePreview->GetDoOpen() && activePreview->GetSaveInfo())
	{
		searchModel->SetLoadedSave(activePreview->TakeSaveInfo());
	}
	else
	{
		searchModel->SetLoadedSave(nullptr);
	}
}

void SearchController::OpenSave(int saveID, int saveDate, std::unique_ptr<VideoBuffer> thumbnail)
{
	delete activePreview;
	Graphics * g = searchView->GetGraphics();
	g->BlendFilledRect(RectSized(Vec2{ XRES/3, WINDOWH-20 }, Vec2{ XRES/3, 20 }), 0x000000_rgb .WithAlpha(150)); //dim the "Page X of Y" a little to make the CopyTextButton more noticeable
	activePreview = new PreviewController(saveID, saveDate, instantOpen ? savePreviewInstant : savePreviewNormal, [this] { OpenSaveDone(); }, std::move(thumbnail));
	activePreview->GetView()->MakeActiveWindow();
}

void SearchController::ClearSelection()
{
	searchModel->ClearSelected();
}

void SearchController::RemoveSelected()
{
	StringBuilder desc;
	desc << "Are you sure you want to delete " << searchModel->GetSelected().size() << " save";
	if(searchModel->GetSelected().size()>1)
		desc << "s";
	desc << "?";
	new ConfirmPrompt("Delete saves", desc.Build(), { [this] {
		removeSelectedC();
	} });
}

void SearchController::removeSelectedC()
{
	class RemoveSavesTask : public Task
	{
		SearchController *c;
		std::vector<int> saves;
	public:
		RemoveSavesTask(std::vector<int> saves_, SearchController *c_) { saves = saves_; c = c_; }
		bool doWork() override
		{
			for (size_t i = 0; i < saves.size(); i++)
			{
				notifyStatus(String::Build("Deleting save [", saves[i], "] ..."));
				auto deleteSaveRequest = std::make_unique<http::DeleteSaveRequest>(saves[i]);
				deleteSaveRequest->Start();
				deleteSaveRequest->Wait();
				try
				{
					deleteSaveRequest->Finish();
				}
				catch (const http::RequestError &ex)
				{
					notifyError(String::Build("Failed to delete [", saves[i], "]: ", ByteString(ex.what()).FromAscii()));
					c->Refresh();
					return false;
				}
				notifyProgress((i + 1) * 100 / saves.size());
			}
			c->Refresh();
			return true;
		}
	};

	std::vector<int> selected = searchModel->GetSelected();
	new TaskWindow("Removing saves", new RemoveSavesTask(selected, this));
	ClearSelection();
	searchModel->UpdateSaveList(searchModel->GetPageNum(), searchModel->GetLastQuery());
}

void SearchController::UnpublishSelected(bool publish)
{
	StringBuilder desc;
	desc << "Are you sure you want to " << (publish ? String("publish ") : String("unpublish ")) << searchModel->GetSelected().size() << " save";
	if (searchModel->GetSelected().size() > 1)
		desc << "s";
	desc << "?";
	new ConfirmPrompt(publish ? String("Publish Saves") : String("Unpublish Saves"), desc.Build(), { [this, publish] {
		unpublishSelectedC(publish);
	} });
}

void SearchController::unpublishSelectedC(bool publish)
{
	class UnpublishSavesTask : public Task
	{
		std::vector<int> saves;
		SearchController *c;
		bool publish;
	public:
		UnpublishSavesTask(std::vector<int> saves_, SearchController *c_, bool publish_) { saves = saves_; c = c_; publish = publish_; }

		void PublishSave(int saveID)
		{
			notifyStatus(String::Build("Publishing save [", saveID, "]"));
			auto publishSaveRequest = std::make_unique<http::PublishSaveRequest>(saveID);
			publishSaveRequest->Start();
			publishSaveRequest->Wait();
			publishSaveRequest->Finish();
		}

		void UnpublishSave(int saveID)
		{
			notifyStatus(String::Build("Unpublishing save [", saveID, "]"));
			auto unpublishSaveRequest = std::make_unique<http::UnpublishSaveRequest>(saveID);
			unpublishSaveRequest->Start();
			unpublishSaveRequest->Wait();
			unpublishSaveRequest->Finish();
		}

		bool doWork() override
		{
			for (size_t i = 0; i < saves.size(); i++)
			{
				try
				{
					if (publish)
					{
						PublishSave(saves[i]);
					}
					else
					{
						UnpublishSave(saves[i]);
					}
				}
				catch (const http::RequestError &ex)
				{
					if (publish) // uses html page so error message will be spam
					{
						notifyError(String::Build("Failed to publish [", saves[i], "], is this save yours?"));
					}
					else
					{
						notifyError(String::Build("Failed to unpublish [", saves[i], "]: ", ByteString(ex.what()).FromAscii()));
					}
					c->Refresh();
					return false;
				}
				notifyProgress((i + 1) * 100 / saves.size());
			}
			c->Refresh();
			return true;
		}
	};

	std::vector<int> selected = searchModel->GetSelected();
	new TaskWindow(publish ? String("Publishing Saves") : String("Unpublishing Saves"), new UnpublishSavesTask(selected, this, publish));
}

void SearchController::FavouriteSelected()
{
	class FavouriteSavesTask : public Task
	{
		std::vector<int> saves;
	public:
		FavouriteSavesTask(std::vector<int> saves_) { saves = saves_; }
		bool doWork() override
		{
			for (size_t i = 0; i < saves.size(); i++)
			{
				notifyStatus(String::Build("Favouring save [", saves[i], "]"));
				auto favouriteSaveRequest = std::make_unique<http::FavouriteSaveRequest>(saves[i], true);
				favouriteSaveRequest->Start();
				favouriteSaveRequest->Wait();
				try
				{
					favouriteSaveRequest->Finish();
				}
				catch (const http::RequestError &ex)
				{
					notifyError(String::Build("Failed to favourite [", saves[i], "]: ", ByteString(ex.what()).FromAscii()));
					return false;
				}
				notifyProgress((i + 1) * 100 / saves.size());
			}
			return true;
		}
	};

	class UnfavouriteSavesTask : public Task
	{
		std::vector<int> saves;
	public:
		UnfavouriteSavesTask(std::vector<int> saves_) { saves = saves_; }
		bool doWork() override
		{
			for (size_t i = 0; i < saves.size(); i++)
			{
				notifyStatus(String::Build("Unfavouring save [", saves[i], "]"));
				auto unfavouriteSaveRequest = std::make_unique<http::FavouriteSaveRequest>(saves[i], false);
				unfavouriteSaveRequest->Start();
				unfavouriteSaveRequest->Wait();
				try
				{
					unfavouriteSaveRequest->Finish();
				}
				catch (const http::RequestError &ex)
				{
					notifyError(String::Build("Failed to unfavourite [", saves[i], "]: ", ByteString(ex.what()).FromAscii()));
					return false;
				}
				notifyProgress((i + 1) * 100 / saves.size());
			}
			return true;
		}
	};

	std::vector<int> selected = searchModel->GetSelected();
	if (!searchModel->GetShowFavourite())
		new TaskWindow("Favouring saves", new FavouriteSavesTask(selected));
	else
		new TaskWindow("Unfavouring saves", new UnfavouriteSavesTask(selected));
	ClearSelection();
}
