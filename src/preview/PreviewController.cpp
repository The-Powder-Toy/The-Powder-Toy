/*
 * PreviewController.cpp
 *
 *  Created on: Jan 21, 2012
 *      Author: Simon
 */

#include <sstream>
#include "client/Client.h"
#include "PreviewController.h"
#include "PreviewView.h"
#include "PreviewModel.h"
#include "PreviewModelException.h"
#include "dialogues/ErrorMessage.h"
#include "Controller.h"

PreviewController::PreviewController(int saveID, ControllerCallback * callback):
	HasExited(false),
	saveId(saveID)
{
	previewModel = new PreviewModel();
	previewView = new PreviewView();
	previewModel->AddObserver(previewView);
	previewView->AttachController(this);

	previewModel->UpdateSave(saveID, 0);

	this->callback = callback;
}

void PreviewController::Update()
{
	try
	{
		previewModel->Update();
	}
	catch (PreviewModelException & e)
	{
		Exit();
		new ErrorMessage("Error", e.what());
	}
	if(previewModel->GetDoOpen() && previewModel->GetSave() && previewModel->GetSave()->GetGameSave())
	{
		Exit();
	}
}

SaveInfo * PreviewController::GetSave()
{
	return previewModel->GetSave();
}

bool PreviewController::GetDoOpen()
{
	return previewModel->GetDoOpen();
}

void PreviewController::DoOpen()
{
	previewModel->SetDoOpen(true);
}

void PreviewController::Report(std::string message)
{
	if(Client::Ref().ReportSave(saveId, message) == RequestOkay)
	{
		Exit();
		new ErrorMessage("Information", "Report submitted"); //TODO: InfoMessage
	}
	else
		new ErrorMessage("Error", "Unable file report");
}

void PreviewController::FavouriteSave()
{
	previewModel->SetFavourite(true);
}

void PreviewController::OpenInBrowser()
{
	if(previewModel->GetSave())
	{
		std::stringstream uriStream;
		uriStream << "http://" << SERVER << "/Browse/View.html?ID=" << previewModel->GetSave()->id;
		OpenURI(uriStream.str());
	}
}

void PreviewController::Exit()
{
	if(ui::Engine::Ref().GetWindow() == previewView)
	{
		ui::Engine::Ref().CloseWindow();
	}
	if(callback)
		callback->ControllerExit();
	HasExited = true;
}

PreviewController::~PreviewController() {
	if(ui::Engine::Ref().GetWindow() == previewView)
	{
		ui::Engine::Ref().CloseWindow();
	}
	delete previewModel;
}

