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
#include "Platform.h"
#include "tasks/Task.h"
#include "tasks/TaskWindow.h"

class SearchController::OpenCallback: public ControllerCallback
{
	SearchController * cc;
public:
	OpenCallback(SearchController * cc_) { cc = cc_; }
	virtual void ControllerExit()
	{
		if(cc->activePreview->GetDoOpen() && cc->activePreview->GetSaveInfo())
		{
			cc->searchModel->SetLoadedSave(cc->activePreview->GetSaveInfo());
		}
		else
		{
			cc->searchModel->SetLoadedSave(NULL);
		}

	}
};

SearchController::SearchController(ControllerCallback * callback):
	activePreview(NULL),
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
		activePreview = NULL;
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
	if(callback)
		callback->ControllerExit();
	//HasExited = true;
}

SearchController::~SearchController()
{
	delete activePreview;
	searchView->CloseActiveWindow();
	delete searchModel;
	delete searchView;
	delete callback;
}

void SearchController::DoSearch(std::string query, bool now)
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

void SearchController::DoSearch2(std::string query)
{
	// calls SearchView function to set textbox text, then calls DoSearch
	searchView->Search(query);
}

void SearchController::Refresh()
{
	doRefresh = true;
}

void SearchController::PrevPage()
{
	if (searchModel->GetPageNum()>1)
		searchModel->UpdateSaveList(searchModel->GetPageNum()-1, searchModel->GetLastQuery());
}

void SearchController::NextPage()
{
	if (searchModel->GetPageNum() < searchModel->GetPageCount())
		searchModel->UpdateSaveList(searchModel->GetPageNum()+1, searchModel->GetLastQuery());
}

void SearchController::SetPage(int page)
{
	if (page != searchModel->GetPageNum() && page > 0 && page <= searchModel->GetPageCount())
		searchModel->UpdateSaveList(page, searchModel->GetLastQuery());
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
	delete activePreview;
	Graphics * g = searchView->GetGraphics();
	g->fillrect(XRES/3, WINDOWH-20, XRES/3, 20, 0, 0, 0, 150); //dim the "Page X of Y" a little to make the CopyTextButton more noticeable
	activePreview = new PreviewController(saveID, instantOpen, new OpenCallback(this));
	activePreview->GetView()->MakeActiveWindow();
}

void SearchController::OpenSave(int saveID, int saveDate)
{
	delete activePreview;
	Graphics * g = searchView->GetGraphics();
	g->fillrect(XRES/3, WINDOWH-20, XRES/3, 20, 0, 0, 0, 150); //dim the "Page X of Y" a little to make the CopyTextButton more noticeable
	activePreview = new PreviewController(saveID, saveDate, instantOpen, new OpenCallback(this));
	activePreview->GetView()->MakeActiveWindow();
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
		SearchController *c;
		std::vector<int> saves;
	public:
		RemoveSavesTask(std::vector<int> saves_, SearchController *c_) { saves = saves_; c = c_; }
		virtual bool doWork()
		{
			for (size_t i = 0; i < saves.size(); i++)
			{
				std::stringstream saveID;
				saveID << "Deleting save [" << saves[i] << "] ...";
 				notifyStatus(saveID.str());
				if (Client::Ref().DeleteSave(saves[i])!=RequestOkay)
				{
 					std::stringstream saveIDF;
					saveIDF << "Failed to delete [" << saves[i] << "]: " << Client::Ref().GetLastError();
					notifyError(saveIDF.str());
					c->Refresh();
					return false;
				}
				notifyProgress((float(i+1)/float(saves.size())*100));
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
	class UnpublishSelectedConfirmation: public ConfirmDialogueCallback {
	public:
		SearchController * c;
		bool publish;
		UnpublishSelectedConfirmation(SearchController * c_, bool publish_) { c = c_; publish = publish_; }
		virtual void ConfirmCallback(ConfirmPrompt::DialogueResult result) {
			if (result == ConfirmPrompt::ResultOkay)
				c->unpublishSelectedC(publish);
		}
		virtual ~UnpublishSelectedConfirmation() { }
	};

	std::stringstream desc;
	desc << "Are you sure you want to " << (publish ? "publish " : "unpublish ") << searchModel->GetSelected().size() << " save";
	if (searchModel->GetSelected().size() > 1)
		desc << "s";
	desc << "?";
	new ConfirmPrompt((publish ? "Publish Saves" : "Unpublish Saves"), desc.str(), new UnpublishSelectedConfirmation(this, publish));
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

		bool PublishSave(int saveID)
		{
			std::stringstream message;
			message << "Publishing save [" << saveID << "]";
			notifyStatus(message.str());
			if (Client::Ref().PublishSave(saveID) != RequestOkay)
				return false;
			return true;
		}

		bool UnpublishSave(int saveID)
		{
			std::stringstream message;
			message << "Unpublishing save [" << saveID << "]";
			notifyStatus(message.str());
			if (Client::Ref().UnpublishSave(saveID) != RequestOkay)
				return false;
			return true;
		}

		virtual bool doWork()
		{
			bool ret;
			for (size_t i = 0; i < saves.size(); i++)
			{
				if (publish)
					ret = PublishSave(saves[i]);
				else
					ret = UnpublishSave(saves[i]);
				if (!ret)
				{
					std::stringstream error;
					if (publish) // uses html page so error message will be spam
						error << "Failed to publish [" << saves[i] << "], is this save yours?";
					else
						error << "Failed to unpublish [" << saves[i] << "]: " + Client::Ref().GetLastError();
					notifyError(error.str());
					c->Refresh();
					return false;
				}
				notifyProgress((float(i+1)/float(saves.size())*100));
			}
			c->Refresh();
			return true;
		}
	};

	std::vector<int> selected = searchModel->GetSelected();
	new TaskWindow((publish ? "Publishing Saves" : "Unpublishing Saves"), new UnpublishSavesTask(selected, this, publish));
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
			for (size_t i = 0; i < saves.size(); i++)
			{
				std::stringstream saveID;
				saveID << "Favouring save [" << saves[i] << "]";
				notifyStatus(saveID.str());
				if (Client::Ref().FavouriteSave(saves[i], true)!=RequestOkay)
				{
					std::stringstream saveIDF;
					saveIDF << "Failed to favourite [" << saves[i] << "]: " + Client::Ref().GetLastError();
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
			for (size_t i = 0; i < saves.size(); i++)
			{
				std::stringstream saveID;
				saveID << "Unfavouring save [" << saves[i] << "]";
				notifyStatus(saveID.str());
				if (Client::Ref().FavouriteSave(saves[i], false)!=RequestOkay)
				{
					std::stringstream saveIDF;
					saveIDF << "Failed to unfavourite [" << saves[i] << "]: " + Client::Ref().GetLastError();
					notifyError(saveIDF.str());
					return false;
				}
				notifyProgress((float(i+1)/float(saves.size())*100));
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
