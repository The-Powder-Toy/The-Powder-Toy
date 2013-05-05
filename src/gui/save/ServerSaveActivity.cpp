#include "ServerSaveActivity.h"
#include "gui/interface/Label.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/Button.h"
#include "gui/interface/Checkbox.h"
#include "client/requestbroker/RequestBroker.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/dialogues/InformationMessage.h"
#include "client/Client.h"
#include "tasks/Task.h"
#include "gui/Style.h"
#include "client/GameSave.h"

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

class ServerSaveActivity::RulesAction: public ui::ButtonAction
{
	ServerSaveActivity * a;
public:
	RulesAction(ServerSaveActivity * a) : a(a) {}
	virtual void ActionCallback(ui::Button * sender)
	{
		a->ShowRules();
	}
};

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
	descriptionField->SetLimit(254);
	descriptionField->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	descriptionField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	AddComponent(descriptionField);

	publishedCheckbox = new ui::Checkbox(ui::Point(8, 45), ui::Point((Size.X/2)-80, 16), "Publish", "");
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

	pausedCheckbox = new ui::Checkbox(ui::Point(160, 45), ui::Point(55, 16), "Paused", "");
	pausedCheckbox->SetChecked(save.GetGameSave()->paused);
	AddComponent(pausedCheckbox);

	ui::Button * cancelButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point((Size.X/2)-75, 16), "Cancel");
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

	ui::Button * RulesButton = new ui::Button(ui::Point((Size.X*3/4)-75, Size.Y-20), ui::Point(150, 16), "Save Uploading Rules");
	RulesButton->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	RulesButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	RulesButton->Appearance.TextInactive = style::Colour::InformationTitle;
	RulesButton->SetActionCallback(new RulesAction(this));
	AddComponent(RulesButton);

	if(save.GetGameSave())
		RequestBroker::Ref().RenderThumbnail(save.GetGameSave(), false, true, (Size.X/2)-16, -1, this);
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
	if(!task->GetSuccess())
	{
		Exit();
		new ErrorMessage("Error", Client::Ref().GetLastError());
	}
	else
	{
		if(callback)
		{
			callback->SaveUploaded(save);
		}
		Exit();
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
				a->Exit();
				a->saveUpload();
			}
		}
		virtual ~PublishConfirmation() { }
	};

	if(nameField->GetText().length())
	{
		if(Client::Ref().GetAuthUser().Username != save.GetUserName() && publishedCheckbox->GetChecked())
		{
			new ConfirmPrompt("Publish", "This save was created by " + save.GetUserName() + ", you're about to publish this under your own name; If you haven't been given permission by the author to do so, please uncheck the publish box, otherwise continue", new PublishConfirmation(this));
		}
		else
		{
			Exit();
			saveUpload();
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
	save.GetGameSave()->paused = pausedCheckbox->GetChecked();

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
	WindowActivity::Exit();
}

void ServerSaveActivity::ShowRules()
{
	const char *rules =
		"These are the rules you should follow when uploading saves to avoid having them deleted or otherwise hidden from public view. If you fail to follow them, don't be surprised if your saves get lousy votes, unpublished, or removed from the front page should they make it there. They may change at any time as new problems arise, and how each rule is handled changes depending on the situation.\n"
		"\n"
		"\bt1. No image plotting.\bw If you use a program to draw out pixels from an image outside of TPT without drawing it by hand, don't be surprised when it gets deleted and you get banned.\n"
		"\bt2. No self voting.\bw This means making more than one account, and then using that account to vote on any save multiple times. We can see this stuff, and people get banned for doing it. Don't do it.\n"
		"\bt3. No hate saves.\bw This means things like shooting Jews or killing Beiber; these will not be allowed.\n"
		"\bt4. No penis drawings.\bw Or any other explicit or non-explicit sex please. We like to think this is a game that kids can play with their family around, don't post anything too inappropriate.\n"
		"\bt5. Don't ask people to vote.\bw If your stuff is awesome, you shouldn't have to beg for popularity to get votes. People tend to downvote when they see vote begging anyway.\n"
		   "- This includes vote signs in the game, drawings of vote arrows, and comments on the save telling people to vote up.\n"
		   "- Gimmicks for getting votes like '100 votes and I'll make a better version' are similarly frowned upon.\n"
		"\bt6. Keep the number of logos and signs to a minimum.\bw They not only slow the game down, but it can also make saves unappealing for people to use. \n"
		   "- Please do not make fake update or similar fake signs either.\n"
		"\bt7. Please don't swear excessively.\bw Saves containing excessive swearing or rude language will be unpublished. Don't make rude or offensive comments either.\n"
		"\bt8. Don't make text only saves.\bw Saves are much better when they actually use some of the features in the game. Text only saves will be removed from the front page should they ever get there.\n"
		   "- Also, element suggestion saves will be removed from the front page. It's recommended you make a thread on the forum instead so you can get actual criticism from other users and devs.\n"
		   "- This is also related to art on the front page. Art saves that only rely on the deco layer are generally removed. Art using elements may stay longer if it's more impressive.\n"
		"\bt9. Don't claim others' work as your own.\bw If you didn't make it, don't resave it for yourself. You can fav. a save instead of publishing a copy if you want to see it later.\n"
		   "- This doesn't mean you can't modify or improve saves; building on the works of others is encouraged. If you give credit to the original author, it is usually OK to resave unless the author specifically prohibits it.\n"
		"\bt10. Do not make laggy saves.\bw If a save is so laggy that it crashes the game for some people, it's just really annoying. Saves that do make it to the front page that purposely lag the game will be demoted.\n"
		"\n"
		"You can report a save breaking any one of these rules, as the moderators are busy in real life too and don't always have the time to search through all saves for these kinds of things. If reporting a copied save, just give the ID of the original, but if not an ID isn't needed.";

	new InformationMessage("Save Uploading Rules", rules, true);
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
		g->draw_image(thumbnail, Position.X+(Size.X/2)+((Size.X/2)-thumbnail->Width)/2, Position.Y+25, 255);
		g->drawrect(Position.X+(Size.X/2)+((Size.X/2)-thumbnail->Width)/2, Position.Y+25, thumbnail->Width, thumbnail->Height, 180, 180, 180, 255);
	}
}

void ServerSaveActivity::OnResponseReady(void * imagePtr)
{
	if(thumbnail)
		delete thumbnail;
	thumbnail = (VideoBuffer *)imagePtr;
}

ServerSaveActivity::~ServerSaveActivity()
{
	RequestBroker::Ref().DetachRequestListener(this);
	if(saveUploadTask)
		delete saveUploadTask;
	if(callback)
		delete callback;
	if(thumbnail)
		delete thumbnail;
}
