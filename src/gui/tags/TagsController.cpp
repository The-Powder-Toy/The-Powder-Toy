#include "TagsController.h"

#include "TagsModel.h"
#include "TagsView.h"

#include "gui/interface/Engine.h"
#include "client/SaveInfo.h"
#include "Controller.h"

TagsController::TagsController(std::function<void ()> onDone_, SaveInfo * save):
	HasDone(false)
{
	tagsModel = new TagsModel();
	tagsView = new TagsView();
	tagsView->AttachController(this);
	tagsModel->AddObserver(tagsView);

	tagsModel->SetSave(save);

	onDone = onDone_;
}

SaveInfo * TagsController::GetSave()
{
	return tagsModel->GetSave();
}

void TagsController::RemoveTag(ByteString tag)
{
	tagsModel->RemoveTag(tag);
}


void TagsController::AddTag(ByteString tag)
{
	tagsModel->AddTag(tag);
}

void TagsController::Exit()
{
	tagsView->CloseActiveWindow();
	if (onDone)
		onDone();
	HasDone = true;
}

TagsController::~TagsController()
{
	tagsView->CloseActiveWindow();
	delete tagsModel;
	delete tagsView;
}

