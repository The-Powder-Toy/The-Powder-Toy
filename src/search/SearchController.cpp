#include <string>
#include "SearchController.h"
#include "SearchModel.h"
#include "SearchView.h"
#include "interface/Panel.h"
#include "preview/PreviewController.h"
#include "client/Client.h"

class SearchController::OpenCallback: public ControllerCallback
{
	SearchController * cc;
public:
	OpenCallback(SearchController * cc_) { cc = cc_; }
	virtual void ControllerExit()
	{
		if(cc->activePreview->GetDoOpen() && cc->activePreview->GetSave())
		{
			cc->searchModel->SetLoadedSave(new Save(*(cc->activePreview->GetSave())));
		}
	}
};

SearchController::SearchController(ControllerCallback * callback):
	activePreview(NULL),
	HasExited(false),
	nextQueryTime(0.0f),
	nextQueryDone(true)
{
	searchModel = new SearchModel();
	searchView = new SearchView();
	searchModel->AddObserver(searchView);
	searchView->AttachController(this);

	searchModel->UpdateSaveList(1, "");

	this->callback = callback;

	//Set up interface
	//windowPanel.AddChild();
}

Save * SearchController::GetLoadedSave()
{
	return searchModel->GetLoadedSave();
}

void SearchController::Update()
{
	if(!nextQueryDone && nextQueryTime < clock())
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
	HasExited = true;
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
}

void SearchController::DoSearch(std::string query)
{
	nextQuery = query;
	nextQueryTime = clock()+(0.6 * CLOCKS_PER_SEC);
	nextQueryDone = false;
	//searchModel->UpdateSaveList(1, query);
}

void SearchController::PrevPage()
{
	if(searchModel->GetPageNum()>1)
		searchModel->UpdateSaveList(searchModel->GetPageNum()-1, searchModel->GetLastQuery());
}

void SearchController::NextPage()
{
	if(searchModel->GetPageNum() <= searchModel->GetPageCount())
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
}

void SearchController::ShowOwn(bool show)
{
	if(Client::Ref().GetAuthUser().ID)
		searchModel->SetShowOwn(show);
	else
		searchModel->SetShowOwn(false);
}

void SearchController::OpenSave(int saveID)
{
	activePreview = new PreviewController(saveID, new OpenCallback(this));
	ui::Engine::Ref().ShowWindow(activePreview->GetView());
}
