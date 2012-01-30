/*
 * SSaveController.cpp
 *
 *  Created on: Jan 29, 2012
 *      Author: Simon
 */

#include "SSaveController.h"

SSaveController::SSaveController(ControllerCallback * callback, Save save):
	HasExited(false)
{
	ssaveView = new SSaveView();
	ssaveView->AttachController(this);
	ssaveModel = new SSaveModel();
	ssaveModel->AddObserver(ssaveView);
	ssaveModel->SetSave(new Save(save));

	this->callback = callback;
}

void SSaveController::UploadSave(std::string saveName, std::string saveDescription, bool publish)
{
	ssaveModel->UploadSave(saveName, saveDescription, publish);
}

Save * SSaveController::GetSave()
{
	return ssaveModel->GetSave();
}

bool SSaveController::GetSaveUploaded()
{
	return ssaveModel->GetSaveUploaded();
}

void SSaveController::Update()
{
	ssaveModel->Update();
}

void SSaveController::Exit()
{
	if(ui::Engine::Ref().GetWindow() == ssaveView)
		ui::Engine::Ref().CloseWindow();
	if(callback)
		callback->ControllerExit();
	HasExited = true;
}

SSaveController::~SSaveController() {
	if(ui::Engine::Ref().GetWindow() == ssaveView)
		ui::Engine::Ref().CloseWindow();
}

