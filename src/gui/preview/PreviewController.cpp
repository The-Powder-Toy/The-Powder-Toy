#include <sstream>
#include "client/Client.h"
#include "PreviewController.h"
#include "PreviewView.h"
#include "PreviewModel.h"
#include "PreviewModelException.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/login/LoginController.h"
#include "Controller.h"

PreviewController::PreviewController(int saveID, int saveDate, bool instant, ControllerCallback * callback):
	HasExited(false),
	saveId(saveID),
	saveDate(saveDate),
	loginWindow(NULL)
{
	previewModel = new PreviewModel();
	previewView = new PreviewView();
	previewModel->AddObserver(previewView);
	previewView->AttachController(this);
	previewModel->SetDoOpen(instant);

	previewModel->UpdateSave(saveID, saveDate);

	if(Client::Ref().GetAuthUser().ID)
	{
		previewModel->SetCommentBoxEnabled(true);
	}

	Client::Ref().AddListener(this);

	this->callback = callback;
}

PreviewController::PreviewController(int saveID, bool instant, ControllerCallback * callback):
	HasExited(false),
	saveId(saveID),
	saveDate(0),
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
	if(previewModel->GetDoOpen() && previewModel->GetSave() && previewModel->GetSave()->GetGameSave())
	{
		Exit();
	}
}

bool PreviewController::SubmitComment(std::string comment)
{
	if(comment.length() < 4)
	{
		new ErrorMessage("Error", "Comment is too short");
		return false;
	}
	else
	{
		RequestStatus status = Client::Ref().AddComment(saveId, comment);
		if(status != RequestOkay)
		{
			new ErrorMessage("Error Submitting comment", Client::Ref().GetLastError());
			return false;
		}
		else
		{
			previewModel->UpdateComments(1);
		}
	}
	return true;
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
	if(previewModel->GetSave() && Client::Ref().GetAuthUser().ID)
	{
		try
		{
			if(previewModel->GetSave()->Favourite)
				previewModel->SetFavourite(false);
			else
				previewModel->SetFavourite(true);
		}
		catch (PreviewModelException & e)
		{
			new ErrorMessage("Error", e.what());
		}
	}
}

void PreviewController::OpenInBrowser()
{
	std::stringstream uriStream;
	uriStream << "http://" << SERVER << "/Browse/View.html?ID=" << saveId;
	OpenURI(uriStream.str());
}

bool PreviewController::NextCommentPage()
{
	if(previewModel->GetCommentsPageNum() < previewModel->GetCommentsPageCount() && previewModel->GetCommentsLoaded() && !previewModel->GetDoOpen())
	{
		previewModel->UpdateComments(previewModel->GetCommentsPageNum()+1);
		return true;
	}
	return false;
}

bool PreviewController::PrevCommentPage()
{
	if(previewModel->GetCommentsPageNum() > 1 && previewModel->GetCommentsLoaded() && !previewModel->GetDoOpen())
	{
		previewModel->UpdateComments(previewModel->GetCommentsPageNum()-1);
		return true;
	}
	return false;
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

