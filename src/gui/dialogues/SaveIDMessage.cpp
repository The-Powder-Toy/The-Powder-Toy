#include "gui/Style.h"
#include "SaveIDMessage.h"
#include "graphics/Graphics.h"
#include "gui/interface/Button.h"
#include "gui/interface/CopyTextButton.h"
#include "gui/interface/Label.h"
#include "Format.h"

SaveIDMessage::SaveIDMessage(int id):
	ui::Window(ui::Point((XRES-244)/2, (YRES-90)/2), ui::Point(244, 90))
{
	int textWidth = Graphics::textwidth("Save ID");
	ui::Label * titleLabel = new ui::Label(ui::Point(4, 5), ui::Point(textWidth+20, 16), "Save ID");
	titleLabel->SetTextColour(style::Colour::InformationTitle);
	titleLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	titleLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(titleLabel);

	textWidth = Graphics::textwidth("Saved Successfully!");
	ui::Label * messageLabel = new ui::Label(ui::Point(4, 24), ui::Point(textWidth+20, 16), "Saved Successfully!");
	messageLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	messageLabel->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	AddComponent(messageLabel);

	textWidth = Graphics::textwidth("Click the box below to copy the save ID");
	ui::Label * copyTextLabel = new ui::Label(ui::Point((Size.X-textWidth-20)/2, 35), ui::Point(textWidth+20, 16), "Click the box below to copy the save id");
	copyTextLabel->SetTextColour(ui::Colour(150, 150, 150));
	copyTextLabel->Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
	AddComponent(copyTextLabel);

	textWidth = Graphics::textwidth(format::NumberToString<int>(id).c_str());
	ui::CopyTextButton * copyTextButton = new ui::CopyTextButton(ui::Point((Size.X-textWidth-10)/2, 50), ui::Point(textWidth+10, 18), format::NumberToString<int>(id), copyTextLabel);
	AddComponent(copyTextButton);

	class DismissAction: public ui::ButtonAction
	{
		SaveIDMessage * message;
	public:
		DismissAction(SaveIDMessage * message_) { message = message_; }
		void ActionCallback(ui::Button * sender)
		{
			message->CloseActiveWindow();
			message->SelfDestruct();
		}
	};

	ui::Button * okayButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(Size.X, 16), "OK");
	okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	okayButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	okayButton->SetActionCallback(new DismissAction(this));
	AddComponent(okayButton);
	// This button has multiple personalities
	SetOkayButton(okayButton);
	SetCancelButton(okayButton);
	
	MakeActiveWindow();
}

void SaveIDMessage::OnDraw()
{
	Graphics * g = GetGraphics();

	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 200, 200, 200, 255);
}

void SaveIDMessage::OnTryExit(ExitMethod method)
{
	CloseActiveWindow();
	SelfDestruct();
}

SaveIDMessage::~SaveIDMessage()
{

}

