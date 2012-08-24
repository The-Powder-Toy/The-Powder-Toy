/*
 * StampsController.cpp
 *
 *  Created on: Mar 29, 2012
 *      Author: Simon
 */
#include <sstream>

#include "client/Client.h"
#include "LocalBrowserController.h"
#include "interface/Engine.h"
#include "dialogues/ConfirmPrompt.h"
#include "tasks/TaskWindow.h"
#include "tasks/Task.h"

#include "LocalBrowserModel.h"
#include "LocalBrowserView.h"

LocalBrowserController::LocalBrowserController(ControllerCallback * callback):
	HasDone(false)
{
	browserModel = new LocalBrowserModel();
	browserView = new LocalBrowserView();
	browserView->AttachController(this);
	browserModel->AddObserver(browserView);

	this->callback = callback;

	browserModel->UpdateSavesList(1);
}

void LocalBrowserController::OpenSave(SaveFile * save)
{
	browserModel->SetSave(save);
}

SaveFile * LocalBrowserController::GetSave()
{
	return browserModel->GetSave();
}

void LocalBrowserController::RemoveSelected()
{
	class RemoveSelectedConfirmation: public ConfirmDialogueCallback {
	public:
		LocalBrowserController * c;
		RemoveSelectedConfirmation(LocalBrowserController * c_) {	c = c_;	}
		virtual void ConfirmCallback(ConfirmPrompt::DialogueResult result) {
			if (result == ConfirmPrompt::ResultOkay)
				c->removeSelectedC();
		}
		virtual ~RemoveSelectedConfirmation() { }
	};

	std::stringstream desc;
	desc << "Are you sure you want to delete " << browserModel->GetSelected().size() << " saves";
	if(browserModel->GetSelected().size()>1)
		desc << "s";
	new ConfirmPrompt("Delete saves", desc.str(), new RemoveSelectedConfirmation(this));
}

void LocalBrowserController::removeSelectedC()
{
	class RemoveSavesTask : public Task
	{
		std::vector<std::string> saves;
		LocalBrowserController * c;
	public:
		RemoveSavesTask(LocalBrowserController * c, std::vector<std::string> saves_) : c(c) { saves = saves_; }
		virtual bool doWork()
		{
			for(int i = 0; i < saves.size(); i++)
			{
				std::stringstream saveName;
				saveName << "Deleting save [" << saves[i] << "] ...";
 				notifyStatus(saveName.str());
 				Client::Ref().DeleteStamp(saves[i]);
				notifyProgress((float(i+1)/float(saves.size())*100));
			}
			return true;
		}
		virtual void after()
		{
			c->RefreshSavesList();
		}
	};

	std::vector<std::string> selected = browserModel->GetSelected();
	new TaskWindow("Removing saves", new RemoveSavesTask(this, selected));
}

void LocalBrowserController::RefreshSavesList()
{
	ClearSelection();
	browserModel->UpdateSavesList(browserModel->GetPageNum());
}

void LocalBrowserController::ClearSelection()
{
	browserModel->ClearSelected();
}

void LocalBrowserController::NextPage()
{
	if(browserModel->GetPageNum() < browserModel->GetPageCount())
		browserModel->UpdateSavesList(browserModel->GetPageNum()+1);
}

void LocalBrowserController::PrevPage()
{
	if(browserModel->GetPageNum()>1)
		browserModel->UpdateSavesList(browserModel->GetPageNum()-1);
}

void LocalBrowserController::Update()
{
	if(browserModel->GetSave())
	{
		Exit();
	}
}

void LocalBrowserController::Selected(std::string saveName, bool selected)
{
	if(selected)
		browserModel->SelectSave(saveName);
	else
		browserModel->DeselectSave(saveName);
}

void LocalBrowserController::Exit()
{
	if(ui::Engine::Ref().GetWindow() == browserView)
		ui::Engine::Ref().CloseWindow();
	if(callback)
		callback->ControllerExit();
	HasDone = true;
}

LocalBrowserController::~LocalBrowserController() {
	if(ui::Engine::Ref().GetWindow() == browserView)
		ui::Engine::Ref().CloseWindow();
	if(callback)
		delete callback;
	delete browserModel;
	delete browserView;
}

