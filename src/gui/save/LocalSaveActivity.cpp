#include "LocalSaveActivity.h"
#include "gui/interface/Label.h"
#include "gui/interface/Textbox.h"
#include "gui/interface/Button.h"
#include "gui/search/Thumbnail.h"
#include "client/requestbroker/RequestBroker.h"
#include "gui/dialogues/ErrorMessage.h"
#include "gui/dialogues/ConfirmPrompt.h"
#include "client/Client.h"
#include "client/GameSave.h"
#include "gui/Style.h"
#include "images.h"

class LocalSaveActivity::CancelAction: public ui::ButtonAction
{
	LocalSaveActivity * a;
public:
	CancelAction(LocalSaveActivity * a) : a(a) {}
	virtual void ActionCallback(ui::Button * sender)
	{
		a->Exit();
	}
};

class LocalSaveActivity::SaveAction: public ui::ButtonAction
{
	LocalSaveActivity * a;
public:
	SaveAction(LocalSaveActivity * a) : a(a) {}
	virtual void ActionCallback(ui::Button * sender)
	{
		a->Save();
	}
};

LocalSaveActivity::LocalSaveActivity(SaveFile save, FileSavedCallback * callback) :
	WindowActivity(ui::Point(-1, -1), ui::Point(220, 200)),
	save(save),
	thumbnail(NULL),
	callback(callback)
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
	cancelButton->SetActionCallback(new CancelAction(this));
	AddComponent(cancelButton);
	SetCancelButton(cancelButton);

	ui::Button * okayButton = new ui::Button(ui::Point(Size.X-76, Size.Y-16), ui::Point(76, 16), "Save");
	okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	okayButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	okayButton->Appearance.TextInactive = style::Colour::InformationTitle;
	okayButton->SetActionCallback(new SaveAction(this));
	AddComponent(okayButton);
	SetOkayButton(okayButton);

	if(save.GetGameSave())
		RequestBroker::Ref().RenderThumbnail(save.GetGameSave(), true, false, Size.X-16, -1, this);
}

void LocalSaveActivity::Save()
{
	class FileOverwriteConfirmation: public ConfirmDialogueCallback {
	public:
		LocalSaveActivity * a;
		std::string filename;
		FileOverwriteConfirmation(LocalSaveActivity * a, std::string finalFilename) : a(a), filename(finalFilename) {}
		virtual void ConfirmCallback(ConfirmPrompt::DialogueResult result) {
			if (result == ConfirmPrompt::ResultOkay)
			{
				a->saveWrite(filename);
			}
		}
		virtual ~FileOverwriteConfirmation() { }
	};

	if(filenameField->GetText().length())
	{
		std::string finalFilename = std::string(LOCAL_SAVE_DIR) + std::string(PATH_SEP) + filenameField->GetText() + ".cps";
		save.SetDisplayName(filenameField->GetText());
		save.SetFileName(finalFilename);
		if(Client::Ref().FileExists(finalFilename))
		{
			new ConfirmPrompt("Overwrite file", "Are you sure you wish to overwrite\n"+finalFilename, new FileOverwriteConfirmation(this, finalFilename));
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

void LocalSaveActivity::saveWrite(std::string finalFilename)
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
	else if (Client::Ref().WriteFile(gameSave->Serialise(), finalFilename))
		new ErrorMessage("Error", "Unable to write save file.");
	else
	{
		callback->FileSaved(&save);
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
		g->draw_image(thumbnail, Position.X+(Size.X-thumbnail->Width)/2, Position.Y+45, 255);
		g->drawrect(Position.X+(Size.X-thumbnail->Width)/2, Position.Y+45, thumbnail->Width, thumbnail->Height, 180, 180, 180, 255);
	}
}

void LocalSaveActivity::OnResponseReady(void * imagePtr, int identifier)
{
	delete thumbnail;
	thumbnail = (VideoBuffer*)imagePtr;
}

LocalSaveActivity::~LocalSaveActivity()
{
	RequestBroker::Ref().DetachRequestListener(this);
	delete thumbnail;
	delete callback;
}
