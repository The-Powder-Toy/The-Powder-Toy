#include <string>
#include "SearchController.h"
#include "SearchModel.h"
#include "SearchView.h"
#include "interface/Panel.h"
#include "preview/PreviewController.h"

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
	HasExited(false)
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
	delete searchModel;
	if(searchView)
		delete searchView;
}

void SearchController::DoSearch(std::string query)
{
	searchModel->UpdateSaveList(1, query);
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
	if(searchModel->GetSort() == "date")
	{
		searchModel->SetSort("votes");
	}
	else
	{
		searchModel->SetSort("date");
	}
}

void SearchController::ShowOwn(bool show)
{
	//TODO: Implement
}

void SearchController::OpenSave(int saveID)
{
	activePreview = new PreviewController(saveID, new OpenCallback(this));
	ui::Engine::Ref().ShowWindow(activePreview->GetView());
}
