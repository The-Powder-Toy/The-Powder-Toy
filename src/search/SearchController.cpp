#include "SearchController.h"
#include "SearchModel.h"
#include "SearchView.h"
#include "interface/Panel.h"

SearchController::SearchController()
{
	searchModel = new SearchModel();
	searchView = new SearchView();
	searchModel->AddObserver(searchView);

	//Set up interface
	//windowPanel.AddChild();
}
