#include "ServerSaveActivity.h"
#include "interface/Label.h"
#include "interface/Textbox.h"
#include "interface/Button.h"
#include "interface/Checkbox.h"
#include "search/Thumbnail.h"
#include "client/ThumbnailBroker.h"
#include "dialogues/ErrorMessage.h"
#include "dialogues/ConfirmPrompt.h"
#include "client/Client.h"
#include "tasks/Task.h"
#include "Style.h"

class ServerSaveActivity::CancelAction: public ui::ButtonAction
{
	ServerSaveActivity * a;
public:
	CancelAction(ServerSaveActivity * a) : a(a) {}
	virtual void ActionCallback(ui::Button * sender)
	{
		a->Exit();
	}
};

class ServerSaveActivity::SaveAction: public ui::ButtonAction
{
	ServerSaveActivity * a;
public:
	SaveAction(ServerSaveActivity * a) : a(a) {}
	virtual void ActionCallback(ui::Button * sender)
	{
		a->Save();
	}
};

//Currently, reading is done on another thread, we can't render outside the main thread due to some bullshit with OpenGL 
class SaveUploadTask: public Task
{
	SaveInfo save;

	virtual void before()
	{

	}

	virtual void after()
	{

	}

	virtual bool doWork()
	{
		notifyProgress(-1);
		return Client::Ref().UploadSave(save) == RequestOkay;
	}

public:
	SaveInfo GetSave()
	{
		return save;
	}

	SaveUploadTask(SaveInfo save):
		save(save)
	{

	}
};

ServerSaveActivity::ServerSaveActivity(SaveInfo save, ServerSaveActivity::SaveUploadedCallback * callback) :
	WindowActivity(ui::Point(-1, -1), ui::Point(440, 200)),
	thumbnail(NULL),
	save(save),
	callback(callback),
	saveUploadTask(NULL)
{
	ui::Label * titleLabel = new ui::Label(ui::Point(4, 5), ui::Point((Size.X/2)-8, 16), "Save to server:");
	titleLabel->SetTextColour(style::Colour::InformationTitle);
	titleLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	titleLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(titleLabel);

	ui::Label * previewLabel = new ui::Label(ui::Point((Size.X/2)+4, 5), ui::Point((Size.X/2)-8, 16), "Preview:");
	previewLabel->SetTextColour(style::Colour::InformationTitle);
	previewLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	previewLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(previewLabel);

	nameField = new ui::Textbox(ui::Point(8, 25), ui::Point((Size.X/2)-16, 16), save.GetName(), "[save name]");
	nameField->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	nameField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	AddComponent(nameField);

	descriptionField = new ui::Textbox(ui::Point(8, 65), ui::Point((Size.X/2)-16, Size.Y-(65+16+4)), save.GetDescription(), "[save description]");
	descriptionField->SetMultiline(true);
	descriptionField->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	descriptionField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	AddComponent(descriptionField);

	publishedCheckbox = new ui::Checkbox(ui::Point(8, 45), ui::Point((Size.X/2)-16, 16), "Publish");
	if(Client::Ref().GetAuthUser().Username != save.GetUserName())
	{
		//Save is not owned by the user, disable by default
		publishedCheckbox->SetChecked(false);	
	}
	else
	{
		//Save belongs to the current user, use published state already set
		publishedCheckbox->SetChecked(save.GetPublished());
	}
	AddComponent(publishedCheckbox);

	ui::Button * cancelButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point((Size.X/2)-50, 16), "Cancel");
	cancelButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	cancelButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	cancelButton->Appearance.BorderInactive = ui::Colour(200, 200, 200);
	cancelButton->SetActionCallback(new CancelAction(this));
	AddComponent(cancelButton);
	SetCancelButton(cancelButton);

	ui::Button * okayButton = new ui::Button(ui::Point((Size.X/2)-76, Size.Y-16), ui::Point(76, 16), "Save");
	okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	okayButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	okayButton->Appearance.TextInactive = style::Colour::InformationTitle;
	okayButton->SetActionCallback(new SaveAction(this));
	AddComponent(okayButton);
	SetOkayButton(okayButton);

	if(save.GetGameSave())
		ThumbnailBroker::Ref().RenderThumbnail(save.GetGameSave(), (Size.X/2)-16, -1, this);
}

ServerSaveActivity::ServerSaveActivity(SaveInfo save, bool saveNow, ServerSaveActivity::SaveUploadedCallback * callback) :
	WindowActivity(ui::Point(-1, -1), ui::Point(200, 50)),
	thumbnail(NULL),
	save(save),
	callback(callback),
	saveUploadTask(NULL)
{
	ui::Label * titleLabel = new ui::Label(ui::Point(0, 0), Size, "Saving to server...");
	titleLabel->SetTextColour(style::Colour::InformationTitle);
	titleLabel->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	titleLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(titleLabel);

	saveUploadTask = new SaveUploadTask(save);
	saveUploadTask->AddTaskListener(this);
	saveUploadTask->Start();
}

void ServerSaveActivity::NotifyDone(Task * task)
{
	Exit();
	if(!task->GetSuccess())
	{
		new ErrorMessage("Error", "Error while saving");
	}
}

void ServerSaveActivity::Save()
{
	class PublishConfirmation: public ConfirmDialogueCallback {
	public:
		ServerSaveActivity * a;
		PublishConfirmation(ServerSaveActivity * a) : a(a) {}
		virtual void ConfirmCallback(ConfirmPrompt::DialogueResult result) {
			if (result == ConfirmPrompt::ResultOkay)
			{
				a->saveUpload();
				a->Exit();
			}
		}
		virtual ~PublishConfirmation() { }
	};

	if(nameField->GetText().length())
	{
		if(Client::Ref().GetAuthUser().Username != save.GetUserName() && publishedCheckbox->GetChecked())
		{
			new ConfirmPrompt("Publish", "This save was created by " + save.GetUserName() + ", you're about to publish this under your own name; If you haven't been given permission by the author to do so, please untick the publish box, otherwise continue", new PublishConfirmation(this));
		}
		else
		{
			saveUpload();
			Exit();
		}
	}
	else
	{
		new ErrorMessage("Error", "You must specify a save name.");
	}
}

void ServerSaveActivity::saveUpload()
{
	save.SetName(nameField->GetText());
	save.SetDescription(descriptionField->GetText());
	save.SetPublished(publishedCheckbox->GetChecked());
	save.SetUserName(Client::Ref().GetAuthUser().Username);
	save.SetID(0);

	if(Client::Ref().UploadSave(save) != RequestOkay)
	{
		new ErrorMessage("Error", "Upload failed with error:\n"+Client::Ref().GetLastError());
	}
	else if(callback)
	{
		callback->SaveUploaded(save);
	}
}

void ServerSaveActivity::Exit()
{
	if(callback)
	{
		delete callback;
		callback = NULL;
	}
	WindowActivity::Exit();
}

void ServerSaveActivity::OnTick(float dt)
{
	if(saveUploadTask)
		saveUploadTask->Poll();
}

void ServerSaveActivity::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;
	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);

	if(Size.X>220)
		g->draw_line(Position.X+(Size.X/2)-1, Position.Y, Position.X+(Size.X/2)-1, Position.Y+Size.Y-1, 255, 255, 255, 255);

	if(thumbnail)
	{
		g->draw_image(thumbnail->Data, Position.X+(Size.X/2)+((Size.X/2)-thumbnail->Size.X)/2, Position.Y+25, thumbnail->Size.X, thumbnail->Size.Y, 255);
		g->drawrect(Position.X+(Size.X/2)+((Size.X/2)-thumbnail->Size.X)/2, Position.Y+25, thumbnail->Size.X, thumbnail->Size.Y, 180, 180, 180, 255);
	}
}

void ServerSaveActivity::OnThumbnailReady(Thumbnail * thumbnail)
{
	this->thumbnail = thumbnail;
}

ServerSaveActivity::~ServerSaveActivity()
{
	if(saveUploadTask)
		delete saveUploadTask;
}