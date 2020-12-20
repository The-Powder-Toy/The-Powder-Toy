#include "LocalSaveActivity.h"

#include "images.h"

#include "client/Client.h"
#include "client/GameSave.h"
#include "client/ThumbnailRendererTask.h"

#include "graphics/Graphics.h"

#include "gui/Style.h"

#include "gui/dialogues/ConfirmPrompt.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/interface/Button.h"
#include "gui/interface/Label.h"
#include "gui/interface/Textbox.h"

LocalSaveActivity::LocalSaveActivity(SaveFile save, OnSaved onSaved_) :
	WindowActivity(ui::Point(-1, -1), ui::Point(220, 200)),
	save(save),
	thumbnailRenderer(nullptr),
	onSaved(onSaved_)
{
	ui::Label * titleLabel = new ui::Label(ui::Point(4, 5), ui::Point(Size.X-8, 16), "Save to computer:");
	titleLabel->SetTextColour(style::Colour::InformationTitle);
	titleLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	titleLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(titleLabel);

	filenameField = new ui::Textbox(ui::Point(8, 25), ui::Point(Size.X-16, 16), save.GetDisplayName(), "[filename]");
	filenameField->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	filenameField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	AddComponent(filenameField);
	FocusComponent(filenameField);

	ui::Button * cancelButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(Size.X-75, 16), "Cancel");
	cancelButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	cancelButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	cancelButton->Appearance.BorderInactive = ui::Colour(200, 200, 200);
	cancelButton->SetActionCallback({ [this] {
		Exit();
	} });
	AddComponent(cancelButton);
	SetCancelButton(cancelButton);

	ui::Button * okayButton = new ui::Button(ui::Point(Size.X-76, Size.Y-16), ui::Point(76, 16), "Save");
	okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	okayButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	okayButton->Appearance.TextInactive = style::Colour::InformationTitle;
	okayButton->SetActionCallback({ [this] {
		Save();
	} });
	AddComponent(okayButton);
	SetOkayButton(okayButton);

	if(save.GetGameSave())
	{
		thumbnailRenderer = new ThumbnailRendererTask(save.GetGameSave(), Size.X-16, -1, false, true, false);
		thumbnailRenderer->Start();
	}
}

void LocalSaveActivity::OnTick(float dt)
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
}

void LocalSaveActivity::Save()
{
	if (filenameField->GetText().Contains('/') || filenameField->GetText().BeginsWith("."))
	{
		new ErrorMessage("Error", "Invalid filename.");
	}
	else if (filenameField->GetText().length())
	{
		ByteString finalFilename = ByteString(LOCAL_SAVE_DIR) + ByteString(PATH_SEP) + filenameField->GetText().ToUtf8() + ".cps";
		save.SetDisplayName(filenameField->GetText());
		save.SetFileName(finalFilename);
		if(Client::Ref().FileExists(finalFilename))
		{
			new ConfirmPrompt("Overwrite file", "Are you sure you wish to overwrite\n"+finalFilename.FromUtf8(), { [this, finalFilename] {
				saveWrite(finalFilename);
			} });
		}
		else
		{
			saveWrite(finalFilename);
		}
	}
	else
	{
		new ErrorMessage("Error", "You must specify a filename.");
	}
}

void LocalSaveActivity::saveWrite(ByteString finalFilename)
{
	Client::Ref().MakeDirectory(LOCAL_SAVE_DIR);
	GameSave *gameSave = save.GetGameSave();
	Json::Value localSaveInfo;
	localSaveInfo["type"] = "localsave";
	localSaveInfo["username"] = Client::Ref().GetAuthUser().Username;
	localSaveInfo["title"] = finalFilename;
	localSaveInfo["date"] = (Json::Value::UInt64)time(NULL);
	Client::Ref().SaveAuthorInfo(&localSaveInfo);
	gameSave->authors = localSaveInfo;
	std::vector<char> saveData = gameSave->Serialise();
	if (saveData.size() == 0)
		new ErrorMessage("Error", "Unable to serialize game data.");
	else if (Client::Ref().WriteFile(saveData, finalFilename))
		new ErrorMessage("Error", "Unable to write save file.");
	else
	{
		if (onSaved)
		{
			onSaved(&save);
		}
		Exit();
	}
}

void LocalSaveActivity::OnDraw()
{
	Graphics * g = GetGraphics();
	g->draw_rgba_image(save_to_disk_image, 0, 0, 0.7f);
	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);

	if(thumbnail)
	{
		g->draw_image(thumbnail.get(), Position.X+(Size.X-thumbnail->Width)/2, Position.Y+45, 255);
		g->drawrect(Position.X+(Size.X-thumbnail->Width)/2, Position.Y+45, thumbnail->Width, thumbnail->Height, 180, 180, 180, 255);
	}
}

LocalSaveActivity::~LocalSaveActivity()
{
	if (thumbnailRenderer)
	{
		thumbnailRenderer->Abandon();
	}
}
