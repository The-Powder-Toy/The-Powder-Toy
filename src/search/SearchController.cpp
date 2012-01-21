#include <string>
#include "SearchController.h"
#include "SearchModel.h"
#include "SearchView.h"
#include "interface/Panel.h"
#include "preview/PreviewController.h"

SearchController::SearchController():
	activePreview(NULL)
{
	searchModel = new SearchModel();
	searchView = new SearchView();
	searchModel->AddObserver(searchView);
	searchView->AttachController(this);

	searchModel->UpdateSaveList(1, "");

	//Set up interface
	//windowPanel.AddChild();
}

SearchController::~SearchController()
{
	if(activePreview)
	{
		ui::Engine::Ref().CloseWindow();
		delete activePreview;
	}
	delete searchModel;
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
	activePreview = new PreviewController(saveID);
	ui::Engine::Ref().ShowWindow(activePreview->GetView());
}
