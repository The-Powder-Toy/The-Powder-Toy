#include "ServerSaveActivity.h"

#include "graphics/Graphics.h"

#include "gui/interface/Label.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/Button.h"
#include "gui/interface/Checkbox.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/dialogues/SaveIDMessage.h"
#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/dialogues/InformationMessage.h"

#include "client/Client.h"
#include "client/ThumbnailRendererTask.h"
#include "client/GameSave.h"

#include "tasks/Task.h"

#include "gui/Style.h"

#include "images.h"

class SaveUploadTask: public Task
{
	SaveInfo save;

	void before() override
	{

	}

	void after() override
	{

	}

	bool doWork() override
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

ServerSaveActivity::ServerSaveActivity(SaveInfo save, OnUploaded onUploaded_) :
	WindowActivity(ui::Point(-1, -1), ui::Point(440, 200)),
	thumbnailRenderer(nullptr),
	save(save),
	onUploaded(onUploaded_),
	saveUploadTask(NULL)
{
	titleLabel = new ui::Label(ui::Point(4, 5), ui::Point((Size.X/2)-8, 16), ""_ascii);
	titleLabel->SetTextColour(style::Colour::InformationTitle);
	titleLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	titleLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(titleLabel);
	CheckName(save.GetName()); //set titleLabel text

	ui::Label * previewLabel = new ui::Label(ui::Point((Size.X/2)+4, 5), ui::Point((Size.X/2)-8, 16), "Preview:"_i18n);
	previewLabel->SetTextColour(style::Colour::InformationTitle);
	previewLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	previewLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(previewLabel);

	nameField = new ui::Textbox(ui::Point(8, 25), ui::Point((Size.X/2)-16, 16), save.GetName(), "[save name]"_i18n);
	nameField->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	nameField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	nameField->SetActionCallback({ [this] { CheckName(nameField->GetText()); } });
	AddComponent(nameField);
	FocusComponent(nameField);

	descriptionField = new ui::Textbox(ui::Point(8, 65), ui::Point((Size.X/2)-16, Size.Y-(65+16+4)), save.GetDescription(), "[save description]"_i18n);
	descriptionField->SetMultiline(true);
	descriptionField->SetLimit(254);
	descriptionField->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	descriptionField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	AddComponent(descriptionField);

	publishedCheckbox = new ui::Checkbox(ui::Point(8, 45), ui::Point((Size.X/2)-80, 16), "Publish"_i18n, ""_ascii);
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

	pausedCheckbox = new ui::Checkbox(ui::Point(160, 45), ui::Point(55, 16), "Paused"_i18n, ""_ascii);
	pausedCheckbox->SetChecked(save.GetGameSave()->paused);
	AddComponent(pausedCheckbox);

	ui::Button * cancelButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point((Size.X/2)-75, 16), "Cancel"_i18n);
	cancelButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	cancelButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	cancelButton->Appearance.BorderInactive = ui::Colour(200, 200, 200);
	cancelButton->SetActionCallback({ [this] {
		Exit();
	} });
	AddComponent(cancelButton);
	SetCancelButton(cancelButton);

	ui::Button * okayButton = new ui::Button(ui::Point((Size.X/2)-76, Size.Y-16), ui::Point(76, 16), "Save"_i18n);
	okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	okayButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	okayButton->Appearance.TextInactive = style::Colour::InformationTitle;
	okayButton->SetActionCallback({ [this] {
		Save();
	} });
	AddComponent(okayButton);
	SetOkayButton(okayButton);

	ui::Button * PublishingInfoButton = new ui::Button(ui::Point((Size.X*3/4)-75, Size.Y-42), ui::Point(150, 16), "Publishing Info"_i18n);
	PublishingInfoButton->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	PublishingInfoButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	PublishingInfoButton->Appearance.TextInactive = style::Colour::InformationTitle;
	PublishingInfoButton->SetActionCallback({ [this] {
		ShowPublishingInfo();
	} });
	AddComponent(PublishingInfoButton);

	ui::Button * RulesButton = new ui::Button(ui::Point((Size.X*3/4)-75, Size.Y-22), ui::Point(150, 16), "Save Uploading Rules"_i18n);
	RulesButton->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	RulesButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	RulesButton->Appearance.TextInactive = style::Colour::InformationTitle;
	RulesButton->SetActionCallback({ [this] {
		ShowRules();
	} });
	AddComponent(RulesButton);

	if (save.GetGameSave())
	{
		thumbnailRenderer = new ThumbnailRendererTask(save.GetGameSave(), (Size.X/2)-16, -1, false, false, true);
		thumbnailRenderer->Start();
	}
}

ServerSaveActivity::ServerSaveActivity(SaveInfo save, bool saveNow, OnUploaded onUploaded_) :
	WindowActivity(ui::Point(-1, -1), ui::Point(200, 50)),
	thumbnailRenderer(nullptr),
	save(save),
	onUploaded(onUploaded_),
	saveUploadTask(NULL)
{
	ui::Label * titleLabel = new ui::Label(ui::Point(0, 0), Size, "Saving to server..."_i18n);
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
		new ErrorMessage("Error"_i18n, Client::Ref().GetLastError());
	}
	else
	{
		if (onUploaded)
		{
			onUploaded(save);
		}
		Exit();
	}
}

void ServerSaveActivity::Save()
{
	if(nameField->GetText().length())
	{
		if(Client::Ref().GetAuthUser().Username != save.GetUserName() && publishedCheckbox->GetChecked())
		{
			auto authorWarning = i18nMulti("This save was created by ", ", you're about to publish this under your own name; If you haven't been given permission by the author to do so, please uncheck the publish box, otherwise continue");
			new ConfirmPrompt("Publish"_i18n, authorWarning[0] + save.GetUserName().FromUtf8() + authorWarning[1], { [this] {
				Exit();
				saveUpload();
			} });
		}
		else
		{
			Exit();
			saveUpload();
		}
	}
	else
	{
		new ErrorMessage("Error"_i18n, "You must specify a save name."_i18n);
	}
}

void ServerSaveActivity::AddAuthorInfo()
{
	Json::Value serverSaveInfo;
	serverSaveInfo["type"] = "save";
	serverSaveInfo["id"] = save.GetID();
	serverSaveInfo["username"] = Client::Ref().GetAuthUser().Username;
	serverSaveInfo["title"] = save.GetName().ToUtf8();
	serverSaveInfo["description"] = save.GetDescription().ToUtf8();
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
		new ErrorMessage("Error"_i18n, "Upload failed with error:\n"_i18n+Client::Ref().GetLastError());
	}
	else if (onUploaded)
	{
		new SaveIDMessage(save.GetID());
		onUploaded(save);
	}
}

void ServerSaveActivity::Exit()
{
	WindowActivity::Exit();
}

void ServerSaveActivity::ShowPublishingInfo()
{
	new InformationMessage("Publishing Info"_i18n, currentLocale->GetSavePublishingInfo(), true);
}

void ServerSaveActivity::ShowRules()
{
	new InformationMessage("Save Uploading Rules"_i18n, currentLocale->GetRules(), true);
}

void ServerSaveActivity::CheckName(String newname)
{
	if (newname.length() && newname == save.GetName() && save.GetUserName() == Client::Ref().GetAuthUser().Username)
		titleLabel->SetText("Modify simulation properties:"_i18n);
	else
		titleLabel->SetText("Upload new simulation:"_i18n);
}

void ServerSaveActivity::OnTick(float dt)
{
	if (thumbnailRenderer)
	{
		thumbnailRenderer->Poll();
		if (thumbnailRenderer->GetDone())
		{
			thumbnail = thumbnailRenderer->Finish();
			thumbnailRenderer = nullptr;
		}
	}

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
		g->draw_image(thumbnail.get(), Position.X+(Size.X/2)+((Size.X/2)-thumbnail->Width)/2, Position.Y+25, 255);
		g->drawrect(Position.X+(Size.X/2)+((Size.X/2)-thumbnail->Width)/2, Position.Y+25, thumbnail->Width, thumbnail->Height, 180, 180, 180, 255);
	}
}

ServerSaveActivity::~ServerSaveActivity()
{
	if (thumbnailRenderer)
	{
		thumbnailRenderer->Abandon();
	}
	delete saveUploadTask;
}
