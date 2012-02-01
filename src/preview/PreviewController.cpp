/*
 * PreviewController.cpp
 *
 *  Created on: Jan 21, 2012
 *      Author: Simon
 */

#include <sstream>
#include "PreviewController.h"
#include "PreviewView.h"
#include "PreviewModel.h"
#include "Controller.h"

PreviewController::PreviewController(int saveID, ControllerCallback * callback):
	HasExited(false)
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
	previewModel->Update();
}

Save * PreviewController::GetSave()
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

