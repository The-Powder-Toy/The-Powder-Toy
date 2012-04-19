/*
 * StampsController.cpp
 *
 *  Created on: Mar 29, 2012
 *      Author: Simon
 */
#include <sstream>
#include <unistd.h>

#include "client/Client.h"
#include "StampsController.h"
#include "interface/Engine.h"
#include "dialogues/ConfirmPrompt.h"
#include "tasks/TaskWindow.h"
#include "tasks/Task.h"

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

void StampsController::RemoveSelected()
{
	class RemoveSelectedConfirmation: public ConfirmDialogueCallback {
	public:
		StampsController * c;
		RemoveSelectedConfirmation(StampsController * c_) {	c = c_;	}
		virtual void ConfirmCallback(ConfirmPrompt::DialogueResult result) {
			if (result == ConfirmPrompt::ResultOkay)
				c->removeSelectedC();
		}
		virtual ~RemoveSelectedConfirmation() { }
	};

	std::stringstream desc;
	desc << "Are you sure you want to delete " << stampsModel->GetSelected().size() << " stamp";
	if(stampsModel->GetSelected().size()>1)
		desc << "s";
	new ConfirmPrompt("Delete stamps", desc.str(), new RemoveSelectedConfirmation(this));
}

void StampsController::removeSelectedC()
{
	class RemoveSavesTask : public Task
	{
		std::vector<std::string> stamps;
	public:
		RemoveSavesTask(std::vector<std::string> stamps_) { stamps = stamps_; }
		virtual void doWork()
		{
			for(int i = 0; i < stamps.size(); i++)
			{
				std::stringstream stampID;
				stampID << "Deleting stamp [" << stamps[i] << "] ...";
 				notifyStatus(stampID.str());
 				Client::Ref().DeleteStamp(stamps[i]);
				usleep(100*1000);
				notifyProgress((float(i+1)/float(stamps.size())*100));
			}
		}
	};

	std::vector<std::string> selected = stampsModel->GetSelected();
	new TaskWindow("Removing saves", new RemoveSavesTask(selected));
	ClearSelection();
	stampsModel->UpdateStampsList(stampsModel->GetPageNum());
}

void StampsController::ClearSelection()
{
	stampsModel->ClearSelected();
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

void StampsController::Selected(std::string stampID, bool selected)
{
	if(selected)
		stampsModel->SelectStamp(stampID);
	else
		stampsModel->DeselectStamp(stampID);
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

