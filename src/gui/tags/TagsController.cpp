#include "TagsController.h"
#include "gui/interface/Engine.h"

#include "TagsModel.h"
#include "TagsView.h"

TagsController::TagsController(ControllerCallback * callback, SaveInfo * save):
	HasDone(false)
{
	tagsModel = new TagsModel();
	tagsView = new TagsView();
	tagsView->AttachController(this);
	tagsModel->AddObserver(tagsView);

	tagsModel->SetSave(save);

	this->callback = callback;
}

SaveInfo * TagsController::GetSave()
{
	return tagsModel->GetSave();
}

void TagsController::RemoveTag(std::string tag)
{
	tagsModel->RemoveTag(tag);
}


void TagsController::AddTag(std::string tag)
{
	tagsModel->AddTag(tag);
}

void TagsController::Exit()
{
	tagsView->CloseActiveWindow();
	if(callback)
		callback->ControllerExit();
	HasDone = true;
}

TagsController::~TagsController()
{
	tagsView->CloseActiveWindow();
	delete tagsModel;
	delete tagsView;
	delete callback;
}

