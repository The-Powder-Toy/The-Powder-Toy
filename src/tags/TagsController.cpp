/*
 * TagsController.cpp
 *
 *  Created on: Mar 5, 2012
 *      Author: Simon
 */

#include "TagsController.h"
#include "interface/Engine.h"

#include "TagsModel.h"
#include "TagsView.h"

TagsController::TagsController(ControllerCallback * callback, Save * save):
	HasDone(false)
{
	tagsModel = new TagsModel();
	tagsView = new TagsView();
	tagsView->AttachController(this);
	tagsModel->AddObserver(tagsView);

	tagsModel->SetSave(save);

	this->callback = callback;
}

Save * TagsController::GetSave()
{
	return tagsModel->GetSave();
}

void TagsController::Exit()
{
	if(ui::Engine::Ref().GetWindow() == tagsView)
		ui::Engine::Ref().CloseWindow();
	if(callback)
		callback->ControllerExit();
	HasDone = true;
}

TagsController::~TagsController() {
	// TODO Auto-generated destructor stub
}

