#include <string>
#include <sstream>
#include "SearchController.h"
#include "SearchModel.h"
#include "SearchView.h"
#include "gui/interface/Panel.h"
#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/preview/PreviewController.h"
#include "client/Client.h"
#include "Misc.h"
#include "tasks/Task.h"
#include "tasks/TaskWindow.h"

class SearchController::OpenCallback: public ControllerCallback
{
	SearchController * cc;
public:
	OpenCallback(SearchController * cc_) { cc = cc_; }
	virtual void ControllerExit()
	{
		if(cc->activePreview->GetDoOpen() && cc->activePreview->GetSave())
		{
			cc->searchModel->SetLoadedSave(cc->activePreview->GetSave());
		}
		else
		{
			cc->searchModel->SetLoadedSave(NULL);
		}

	}
};

SearchController::SearchController(ControllerCallback * callback):
	activePreview(NULL),
	HasExited(false),
	nextQueryTime(0.0f),
	nextQueryDone(true),
	instantOpen(false),
	searchModel(NULL)
{
	searchModel = new SearchModel();
	searchView = new SearchView();
	searchModel->AddObserver(searchView);
	searchView->AttachController(this);

	searchModel->UpdateSaveList(1, "");

	this->callback = callback;
}

SaveInfo * SearchController::GetLoadedSave()
{
	return searchModel->GetLoadedSave();
}

void SearchController::ReleaseLoadedSave()
{
	searchModel->SetLoadedSave(NULL);
}

void SearchController::Update()
{
	if(!nextQueryDone && nextQueryTime < gettime())
	{
		nextQueryDone = true;
		searchModel->UpdateSaveList(1, nextQuery);
	}
	searchModel->Update();
	if(activePreview && activePreview->HasExited)
	{
		delete activePreview;
		activePreview = NULL;
		if(searchModel->GetLoadedSave())
		{
			Exit();
		}
	}
}

void SearchController::Exit()
{
	if(ui::Engine::Ref().GetWindow() == searchView)
	{
		ui::Engine::Ref().CloseWindow();
	}
	if(callback)
		callback->ControllerExit();
	//HasExited = true;
}

SearchController::~SearchController()
{
	if(activePreview)
		delete activePreview;
	if(ui::Engine::Ref().GetWindow() == searchView)
	{
		ui::Engine::Ref().CloseWindow();
	}
	delete searchModel;
	delete searchView;
	delete callback;
}

void SearchController::DoSearch(std::string query, bool now)
{
	nextQuery = query;
	if(!now)
	{
		nextQueryTime = gettime()+600;
		nextQueryDone = false;
	}
	else
	{
		nextQueryDone = true;
		searchModel->UpdateSaveList(1, nextQuery);
	}
	//searchModel->UpdateSaveList(1, query);
}

void SearchController::PrevPage()
{
	if(searchModel->GetPageNum()>1)
		searchModel->UpdateSaveList(searchModel->GetPageNum()-1, searchModel->GetLastQuery());
}

void SearchController::NextPage()
{
	if(searchModel->GetPageNum() < searchModel->GetPageCount())
		searchModel->UpdateSaveList(searchModel->GetPageNum()+1, searchModel->GetLastQuery());
}

void SearchController::ChangeSort()
{
	if(searchModel->GetSort() == "new")
	{
		searchModel->SetSort("best");
	}
	else
	{
		searchModel->SetSort("new");
	}
	searchModel->UpdateSaveList(1, searchModel->GetLastQuery());
}

void SearchController::ShowOwn(bool show)
{
	if(Client::Ref().GetAuthUser().ID)
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
	if(Client::Ref().GetAuthUser().ID)
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
	if(!Client::Ref().GetAuthUser().ID)
		return;

	if(selected)
		searchModel->SelectSave(saveID);
	else
		searchModel->DeselectSave(saveID);
}

void SearchController::InstantOpen(bool instant)
{
	instantOpen = instant;
}

void SearchController::OpenSave(int saveID)
{
	if(activePreview)
		delete activePreview;
	Graphics * g = ui::Engine::Ref().g;
	g->fillrect(XRES/3, WINDOWH-20, XRES/3, 20, 0, 0, 0, 150); //dim the "Page X of Y" a little to make the CopyTextButton more noticeable
	activePreview = new PreviewController(saveID, instantOpen, new OpenCallback(this));
	ui::Engine::Ref().ShowWindow(activePreview->GetView());
}

void SearchController::OpenSave(int saveID, int saveDate)
{
	if(activePreview)
		delete activePreview;
	Graphics * g = ui::Engine::Ref().g;
	g->fillrect(XRES/3, WINDOWH-20, XRES/3, 20, 0, 0, 0, 150); //dim the "Page X of Y" a little to make the CopyTextButton more noticeable
	activePreview = new PreviewController(saveID, saveDate, instantOpen, new OpenCallback(this));
	ui::Engine::Ref().ShowWindow(activePreview->GetView());
}

void SearchController::ClearSelection()
{
	searchModel->ClearSelected();
}

void SearchController::RemoveSelected()
{
	class RemoveSelectedConfirmation: public ConfirmDialogueCallback {
	public:
		SearchController * c;
		RemoveSelectedConfirmation(SearchController * c_) {	c = c_;	}
		virtual void ConfirmCallback(ConfirmPrompt::DialogueResult result) {
			if (result == ConfirmPrompt::ResultOkay)
				c->removeSelectedC();
		}
		virtual ~RemoveSelectedConfirmation() { }
	};

	std::stringstream desc;
	desc << "Are you sure you want to delete " << searchModel->GetSelected().size() << " save";
	if(searchModel->GetSelected().size()>1)
		desc << "s";
	desc << "?";
	new ConfirmPrompt("Delete saves", desc.str(), new RemoveSelectedConfirmation(this));
}

void SearchController::removeSelectedC()
{
	class RemoveSavesTask : public Task
	{
		std::vector<int> saves;
	public:
		RemoveSavesTask(std::vector<int> saves_) { saves = saves_; }
		virtual bool doWork()
		{
			for(int i = 0; i < saves.size(); i++)
			{
				std::stringstream saveID;
				saveID << "Deleting save [" << saves[i] << "] ...";
 				notifyStatus(saveID.str());
 				if(Client::Ref().DeleteSave(saves[i])!=RequestOkay)
				{
 					std::stringstream saveIDF;
 					saveIDF << "\boFailed to delete [" << saves[i] << "] ...";
					notifyStatus(saveIDF.str());
				}
				notifyProgress((float(i+1)/float(saves.size())*100));
			}
			return true;
		}
	};

	std::vector<int> selected = searchModel->GetSelected();
	new TaskWindow("Removing saves", new RemoveSavesTask(selected));
	ClearSelection();
	searchModel->UpdateSaveList(searchModel->GetPageNum(), searchModel->GetLastQuery());
}

void SearchController::UnpublishSelected()
{
	class UnpublishSelectedConfirmation: public ConfirmDialogueCallback {
	public:
		SearchController * c;
		UnpublishSelectedConfirmation(SearchController * c_) {	c = c_;	}
		virtual void ConfirmCallback(ConfirmPrompt::DialogueResult result) {
			if (result == ConfirmPrompt::ResultOkay)
				c->unpublishSelectedC();
		}
		virtual ~UnpublishSelectedConfirmation() { }
	};

	std::stringstream desc;
	desc << "Are you sure you want to hide " << searchModel->GetSelected().size() << " save";
	if(searchModel->GetSelected().size()>1)
		desc << "s";
	desc << "?";
	new ConfirmPrompt("Unpublish saves", desc.str(), new UnpublishSelectedConfirmation(this));
}

void SearchController::unpublishSelectedC()
{
	class UnpublishSavesTask : public Task
	{
		std::vector<int> saves;
	public:
		UnpublishSavesTask(std::vector<int> saves_) { saves = saves_; }
		virtual bool doWork()
		{
			for(int i = 0; i < saves.size(); i++)
			{
				std::stringstream saveID;
				saveID << "Hiding save [" << saves[i] << "]";
 				notifyStatus(saveID.str());
 				if(Client::Ref().UnpublishSave(saves[i])!=RequestOkay)
				{
 					std::stringstream saveIDF;
					saveIDF << "\boFailed to hide [" << saves[i] << "], is this save yours?";
					notifyError(saveIDF.str());
					return false;
				}
				notifyProgress((float(i+1)/float(saves.size())*100));
			}
			return true;
		}
	};

	std::vector<int> selected = searchModel->GetSelected();
	new TaskWindow("Unpublishing saves", new UnpublishSavesTask(selected));
	ClearSelection();
	searchModel->UpdateSaveList(searchModel->GetPageNum(), searchModel->GetLastQuery());
}

void SearchController::FavouriteSelected()
{
	class FavouriteSavesTask : public Task
	{
		std::vector<int> saves;
	public:
		FavouriteSavesTask(std::vector<int> saves_) { saves = saves_; }
		virtual bool doWork()
		{
			for(int i = 0; i < saves.size(); i++)
			{
				std::stringstream saveID;
				saveID << "Favouring save [" << saves[i] << "]";
				notifyStatus(saveID.str());
				if(Client::Ref().FavouriteSave(saves[i], true)!=RequestOkay)
				{
					std::stringstream saveIDF;
					saveIDF << "\boFailed to favourite [" << saves[i] << "], are you logged in?";
					notifyError(saveIDF.str());
					return false;
				}
				notifyProgress((float(i+1)/float(saves.size())*100));
			}
			return true;
		}
	};

	class UnfavouriteSavesTask : public Task
	{
		std::vector<int> saves;
	public:
		UnfavouriteSavesTask(std::vector<int> saves_) { saves = saves_; }
		virtual bool doWork()
		{
			for(int i = 0; i < saves.size(); i++)
			{
				std::stringstream saveID;
				saveID << "Unfavouring save [" << saves[i] << "]";
				notifyStatus(saveID.str());
				if(Client::Ref().FavouriteSave(saves[i], false)!=RequestOkay)
				{
					std::stringstream saveIDF;
					saveIDF << "\boFailed to unfavourite [" << saves[i] << "], are you logged in?";
					notifyError(saveIDF.str());
					return false;
				}
				notifyProgress((float(i+1)/float(saves.size())*100));
			}
			return true;
		}
	};

	std::vector<int> selected = searchModel->GetSelected();
	if(!searchModel->GetShowFavourite())
		new TaskWindow("Favouring saves", new FavouriteSavesTask(selected));
	else
		new TaskWindow("Unfavouring saves", new UnfavouriteSavesTask(selected));
	ClearSelection();
}
