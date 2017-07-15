#include "ServerSaveActivity.h"
#include "graphics/Graphics.h"
#include "gui/interface/Label.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/Button.h"
#include "gui/interface/Checkbox.h"
#include "client/requestbroker/RequestBroker.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/dialogues/SaveIDMessage.h"
#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/dialogues/InformationMessage.h"
#include "client/Client.h"
#include "tasks/Task.h"
#include "gui/Style.h"
#include "client/GameSave.h"
#include "images.h"

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

class ServerSaveActivity::PublishingAction: public ui::ButtonAction
{
	ServerSaveActivity * a;
public:
	PublishingAction(ServerSaveActivity * a) : a(a) {}
	virtual void ActionCallback(ui::Button * sender)
	{
		a->ShowPublishingInfo();
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

class ServerSaveActivity::NameChangedAction: public ui::TextboxAction
{
public:
	ServerSaveActivity * a;
	NameChangedAction(ServerSaveActivity * a) : a(a) {}
	virtual void TextChangedCallback(ui::Textbox * sender) {
		a->CheckName(sender->GetText());
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
	titleLabel = new ui::Label(ui::Point(4, 5), ui::Point((Size.X/2)-8, 16), "");
	titleLabel->SetTextColour(style::Colour::InformationTitle);
	titleLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	titleLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(titleLabel);
	CheckName(save.GetName()); //set titleLabel text

	ui::Label * previewLabel = new ui::Label(ui::Point((Size.X/2)+4, 5), ui::Point((Size.X/2)-8, 16), "Preview:");
	previewLabel->SetTextColour(style::Colour::InformationTitle);
	previewLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	previewLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(previewLabel);

	nameField = new ui::Textbox(ui::Point(8, 25), ui::Point((Size.X/2)-16, 16), save.GetName(), "[save name]");
	nameField->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	nameField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	nameField->SetActionCallback(new NameChangedAction(this));
	AddComponent(nameField);
	FocusComponent(nameField);

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

	ui::Button * PublishingInfoButton = new ui::Button(ui::Point((Size.X*3/4)-75, Size.Y-42), ui::Point(150, 16), "Publishing Info");
	PublishingInfoButton->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	PublishingInfoButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	PublishingInfoButton->Appearance.TextInactive = style::Colour::InformationTitle;
	PublishingInfoButton->SetActionCallback(new PublishingAction(this));
	AddComponent(PublishingInfoButton);

	ui::Button * RulesButton = new ui::Button(ui::Point((Size.X*3/4)-75, Size.Y-22), ui::Point(150, 16), "Save Uploading Rules");
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

	AddAuthorInfo();

	saveUploadTask = new SaveUploadTask(this->save);
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

void ServerSaveActivity::AddAuthorInfo()
{
	Json::Value serverSaveInfo;
	serverSaveInfo["type"] = "save";
	serverSaveInfo["id"] = save.GetID();
	serverSaveInfo["username"] = Client::Ref().GetAuthUser().Username;
	serverSaveInfo["title"] = save.GetName();
	serverSaveInfo["description"] = save.GetDescription();
	serverSaveInfo["published"] = (int)save.GetPublished();
	serverSaveInfo["date"] = (Json::Value::UInt64)time(NULL);
	Client::Ref().SaveAuthorInfo(&serverSaveInfo);
	save.GetGameSave()->authors = serverSaveInfo;
}

void ServerSaveActivity::saveUpload()
{
	save.SetName(nameField->GetText());
	save.SetDescription(descriptionField->GetText());
	save.SetPublished(publishedCheckbox->GetChecked());
	save.SetUserName(Client::Ref().GetAuthUser().Username);
	save.SetID(0);
	save.GetGameSave()->paused = pausedCheckbox->GetChecked();
	AddAuthorInfo();

	if(Client::Ref().UploadSave(save) != RequestOkay)
	{
		new ErrorMessage("Error", "Upload failed with error:\n"+Client::Ref().GetLastError());
	}
	else if(callback)
	{
		new SaveIDMessage(save.GetID());
		callback->SaveUploaded(save);
	}
}

void ServerSaveActivity::Exit()
{
	WindowActivity::Exit();
}

void ServerSaveActivity::ShowPublishingInfo()
{
	const char *info =
		"In The Powder Toy, one can save simulations to their account in two privacy levels: Published and unpublished. You can choose which one by checking or unchecking the 'publish' checkbox. Saves are unpublished by default, so if you do not check publish nobody will be able to see your saves.\n"
		"\n"
		"\btPublished saves\bw will appear on the 'By Date' feed and will be seen by many people. These saves also contribute to your Average Score, which is displayed publicly on your profile page on the website. Publish saves that you want people to see so they can comment and vote on.\n"
		"\btUnpublished saves\bw will not be shown on the 'By Date' feed. These will not contribute to your Average Score. They are not completely private though, as anyone who knows the save id will be able to view it. You can give the save id out to show specific people the save but not allow just everyone to see it.\n"
		"\n"
		"To quickly resave a save, open it and click the left side of the split resave button to \bt'Reupload the current simulation'\bw. If you want to change the description or change the published status, you can click the right side to \bt'Modify simulation properties'\bw. Note that you can't change the name of saves; this will create an entirely new save with no comments, votes, or tags; separate from the original.\n"
		"You may want to publish an unpublished save after it is finished, or to unpublish some currently published ones. You can do this by opening the save, selecting the 'Modify simulation properties' button, and changing the published status there. You can also \btunpublish or delete saves\bw by selecting them in the 'my own' section of the browser and clicking either one of the buttons that appear on bottom.\n"
		"If a save is under a week old and gains popularity fast, it will be automatically placed on the \btfront page\bw. Only published saves will be able to get here. Moderators can also choose to promote any save onto the front page, but this happens rarely. They can also demote any save from the front page that breaks a rule or they feel doesn't belong.\n"
		"Once you make a save, you can resave it as many times as you want. A short previous \btsave history\bw is saved, just right click any save in the save browser and select 'View History' to view it. This is useful for when you accidentally save something you didn't mean to and want to go back to the old version.\n"
		;

	new InformationMessage("Publishing Info", info, true);
}

void ServerSaveActivity::ShowRules()
{
	const char *rules =
		"These are the rules you should follow when uploading saves to avoid having them deleted or otherwise hidden from public view. If you fail to follow them, don't be surprised if your saves get lousy votes, unpublished, or removed from the front page should they make it there. These rules may change at any time as new problems arise, and how each rule is handled changes depending on the situation.\n"
		"\n"
		"\bt1. No image plotting.\bw If you use a program to draw out pixels from an image outside of TPT without drawing it by hand, don't be surprised when the save gets deleted and you get banned.\n"
		"\bt2. No self voting.\bw This means making more than one account, and then using that account to vote on any save multiple times. We can see this stuff, and people get banned for doing it. Don't do it.\n"
		"\bt3. No hate saves.\bw This means things like shooting Jews or killing Bieber; these are not allowed.\n"
		"\bt4. No penis drawings.\bw Or any other explicit or non-explicit sex please. We like to think this is a game that people can play with their family around. Don't post anything too inappropriate.\n"
		"\bt5. Don't ask people to vote.\bw If your stuff is awesome, you shouldn't have to beg for popularity to get votes. Do not have anything relating to or mentioning votes inside the actual save.\n"
		   "- This includes vote signs in the game, drawings of vote arrows, or almost any mention of voting in the save. If you want to thank people for the votes they have given, that can be a small note in the description or comments, but don't make it excessive either. Testing this rule to see how far you can go is a bad idea and may get your save unpublished.\n"
		   "- Gimmicks for getting votes like \"100 votes and I'll make a better version\" are similarly frowned upon.\n"
		"\bt6. Keep the number of logos and signs to a minimum.\bw They not only slow the game down, but can also make saves unappealing for people to use. \n"
		   "- If you link to more then 3 unrelated saves using link signs, this is just too much advertising, and the save will probably be removed from the front page if it gets there.\n"
		   "- Please do not make fake update signs or similar fake notification signs either.\n"
		"\bt7. Please don't swear excessively.\bw Saves containing excessive swearing or rude language will be unpublished. Don't make rude or offensive comments either.\n"
		"\bt8. Don't make text-only saves.\bw Saves are much better when they actually use some of the features in the game. Text-only saves will be removed from the front page should they ever get there.\n"
		   "- Also, element suggestion saves will be removed from the front page. It's recommended that you make a thread on the forum instead so you can get actual criticism from developers and other users.\n"
		   "- This is also related to art on the front page. Art saves that only rely on the deco layer are generally removed. Art using elements may stay longer if it's more impressive.\n"
		"\bt9. Don't claim others' work as your own.\bw If you didn't make it, don't resave it for yourself. You can favorite a save instead of publishing a copy if you want to see it later.\n"
		   "- This doesn't mean you can't modify or improve saves; building on the works of others is encouraged. If you give credit to the original author, it is usually OK to resave unless the author specifically prohibits it.\n"
		"\bt10. Don't make laggy saves.\bw If a save is so laggy that it crashes the game for some people, it's just really annoying. Saves that do make it to the front page that purposely lag the game will be demoted.\n"
		"\n"
		"You can report a save breaking any one of these rules, as the moderators are busy in real life too and don't always have the time to search through all saves for these kinds of things. If reporting a copied save, put the save ID of the save it was stolen from in the report.";

	new InformationMessage("Save Uploading Rules", rules, true);
}

void ServerSaveActivity::CheckName(std::string newname)
{
	if (newname.length() && newname == save.GetName() && save.GetUserName() == Client::Ref().GetAuthUser().Username)
		titleLabel->SetText("Modify simulation properties:");
	else
		titleLabel->SetText("Upload new simulation:");
}

void ServerSaveActivity::OnTick(float dt)
{
	if(saveUploadTask)
		saveUploadTask->Poll();
}

void ServerSaveActivity::OnDraw()
{
	Graphics * g = GetGraphics();
	g->draw_rgba_image(save_to_server_image, -10, 0, 0.7f);
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

void ServerSaveActivity::OnResponseReady(void * imagePtr, int identifier)
{
	delete thumbnail;
	thumbnail = (VideoBuffer *)imagePtr;
}

ServerSaveActivity::~ServerSaveActivity()
{
	RequestBroker::Ref().DetachRequestListener(this);
	delete saveUploadTask;
	delete callback;
	delete thumbnail;
}
