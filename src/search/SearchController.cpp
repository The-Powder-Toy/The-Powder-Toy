#include <string>
#include "SearchController.h"
#include "SearchModel.h"
#include "SearchView.h"
#include "interface/Panel.h"

SearchController::SearchController()
{
	searchModel = new SearchModel();
	searchView = new SearchView();
	searchModel->AddObserver(searchView);
	searchView->AttachController(this);

	searchModel->UpdateSaveList("");

	//Set up interface
	//windowPanel.AddChild();
}

void SearchController::DoSearch(std::string query)
{
	searchModel->UpdateSaveList(query);
}
