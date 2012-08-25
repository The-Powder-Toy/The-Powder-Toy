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
#include "login/LoginController.h"
#include "Controller.h"

PreviewController::PreviewController(int saveID, ControllerCallback * callback):
	HasExited(false),
	saveId(saveID),
	loginWindow(NULL)
{
	previewModel = new PreviewModel();
	previewView = new PreviewView();
	previewModel->AddObserver(previewView);
	previewView->AttachController(this);

	previewModel->UpdateSave(saveID, 0);

	if(Client::Ref().GetAuthUser().ID)
	{
		previewModel->SetCommentBoxEnabled(true);
	}

	Client::Ref().AddListener(this);

	this->callback = callback;
}

void PreviewController::Update()
{
	if(loginWindow && loginWindow->HasExited == true)
	{
		delete loginWindow;
		loginWindow = NULL;
	}

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

void PreviewController::SubmitComment(std::string comment)
{
	if(comment.length() < 4)
	{
		new ErrorMessage("Error", "Comment is too short");
	}
	else
	{
		RequestStatus status = Client::Ref().AddComment(saveId, comment);
		if(status != RequestOkay)
		{
			new ErrorMessage("Error Submitting comment", Client::Ref().GetLastError());	
		}
		else
		{
			previewModel->UpdateComments(1);
		}
	}
}

void PreviewController::ShowLogin()
{
	loginWindow = new LoginController();
	ui::Engine::Ref().ShowWindow(loginWindow->GetView());
}

void PreviewController::NotifyAuthUserChanged(Client * sender)
{
	previewModel->SetCommentBoxEnabled(sender->GetAuthUser().ID);
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
	std::stringstream uriStream;
	uriStream << "http://" << SERVER << "/Browse/View.html?ID=" << saveId;
	OpenURI(uriStream.str());
}

void PreviewController::NextCommentPage()
{
	if(previewModel->GetCommentsPageNum() < previewModel->GetCommentsPageCount() && previewModel->GetCommentsLoaded())
		previewModel->UpdateComments(previewModel->GetCommentsPageNum()+1);
}

void PreviewController::PrevCommentPage()
{
	if(previewModel->GetCommentsPageNum()>1 && previewModel->GetCommentsLoaded())
		previewModel->UpdateComments(previewModel->GetCommentsPageNum()-1);
}

void PreviewController::Exit()
{
	if(ui::Engine::Ref().GetWindow() == previewView)
	{
		ui::Engine::Ref().CloseWindow();
	}
	HasExited = true;
	if(callback)
		callback->ControllerExit();
}

PreviewController::~PreviewController() {
	if(ui::Engine::Ref().GetWindow() == previewView)
	{
		ui::Engine::Ref().CloseWindow();
	}
	Client::Ref().RemoveListener(this);
	delete previewModel;
	delete previewView;
	if(callback)
		delete callback;
}

