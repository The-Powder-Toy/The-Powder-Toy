#ifdef WIN
#include <direct.h>
#else
#include <sys/stat.h>
#endif
#include "LocalSaveActivity.h"
#include "interface/Label.h"
#include "interface/Textbox.h"
#include "interface/Button.h"
#include "search/Thumbnail.h"
#include "client/ThumbnailBroker.h"
#include "dialogues/ErrorMessage.h"
#include "dialogues/ConfirmPrompt.h"
#include "client/Client.h"
#include "Style.h"

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

LocalSaveActivity::LocalSaveActivity(SaveFile save) :
	WindowActivity(ui::Point(-1, -1), ui::Point(220, 200)),
	thumbnail(NULL),
	save(save)
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
		ThumbnailBroker::Ref().RenderThumbnail(save.GetGameSave(), Size.X-16, -1, this);
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
				a->Exit();
			}
		}
		virtual ~FileOverwriteConfirmation() { }
	};

	if(filenameField->GetText().length())
	{
		std::string finalFilename = std::string(LOCAL_SAVE_DIR) + std::string(PATH_SEP) + filenameField->GetText() + ".cps";
		if(Client::Ref().FileExists(finalFilename))
		{
			new ConfirmPrompt("Overwrite file", "Are you sure you wish to overwrite\n"+finalFilename, new FileOverwriteConfirmation(this, finalFilename));
		}
		else
		{
			saveWrite(finalFilename);
			Exit();
		}
	}
	else
	{
		new ErrorMessage("Error", "You must specify a filename.");
	}
}

void LocalSaveActivity::saveWrite(std::string finalFilename)
{
#ifdef WIN
	_mkdir(LOCAL_SAVE_DIR);
#else
	mkdir(LOCAL_SAVE_DIR, 0755);
#endif
	Client::Ref().WriteFile(save.GetGameSave()->Serialise(), finalFilename);
}

void LocalSaveActivity::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;
	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 255, 255, 255, 255);

	if(thumbnail)
	{
		g->draw_image(thumbnail->Data, Position.X+(Size.X-thumbnail->Size.X)/2, Position.Y+45, thumbnail->Size.X, thumbnail->Size.Y, 255);
		g->drawrect(Position.X+(Size.X-thumbnail->Size.X)/2, Position.Y+45, thumbnail->Size.X, thumbnail->Size.Y, 180, 180, 180, 255);
	}
}

void LocalSaveActivity::OnThumbnailReady(Thumbnail * thumbnail)
{
	this->thumbnail = thumbnail;
}

LocalSaveActivity::~LocalSaveActivity()
{

}