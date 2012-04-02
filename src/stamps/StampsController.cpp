/*
 * StampsController.cpp
 *
 *  Created on: Mar 29, 2012
 *      Author: Simon
 */

#include "StampsController.h"
#include "interface/Engine.h"

#include "StampsModel.h"
#include "StampsView.h"

StampsController::StampsController(ControllerCallback * callback):
	HasDone(false)
{
	stampsModel = new StampsModel();
	stampsView = new StampsView();
	stampsView->AttachController(this);
	stampsModel->AddObserver(stampsView);

	this->callback = callback;

	stampsModel->UpdateStampsList(1);
}

void StampsController::OpenStamp(Save * stamp)
{
	stampsModel->SetStamp(stamp);
}

Save * StampsController::GetStamp()
{
	return stampsModel->GetStamp();
}

void StampsController::NextPage()
{
	if(stampsModel->GetPageNum()>1)
		stampsModel->UpdateStampsList(stampsModel->GetPageNum()-1);
}

void StampsController::PrevPage()
{
	if(stampsModel->GetPageNum() <= stampsModel->GetPageCount())
		stampsModel->UpdateStampsList(stampsModel->GetPageNum()+1);
}

void StampsController::Update()
{
	if(stampsModel->GetStamp())
	{
		Exit();
	}
}

void StampsController::Exit()
{
	if(ui::Engine::Ref().GetWindow() == stampsView)
		ui::Engine::Ref().CloseWindow();
	if(callback)
		callback->ControllerExit();
	HasDone = true;
}

StampsController::~StampsController() {
	// TODO Auto-generated destructor stub
}

