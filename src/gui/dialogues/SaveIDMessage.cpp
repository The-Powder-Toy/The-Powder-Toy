#include "gui/Style.h"
#include "SaveIDMessage.h"
#include "gui/interface/Button.h"
#include "gui/interface/Label.h"
#include "PowderToy.h"

class CopyTextButton : public ui::Button
{
	ui::Label *copyTextLabel;
public:
	CopyTextButton(ui::Point position, ui::Point size, std::string buttonText, ui::Label *copyTextLabel_):
		Button(position, size, buttonText)
	{
		copyTextLabel = copyTextLabel_;
	}

	virtual void OnMouseClick(int x, int y, unsigned int button)
	{
		ui::Button::OnMouseClick(x, y, button);
		ClipboardPush((char*)ButtonText.c_str());

		int textWidth = Graphics::textwidth("Copied!");
		copyTextLabel->SetText("Copied!");
		copyTextLabel->Position = ui::Point(Position.X+(Size.X-textWidth)/2-4, copyTextLabel->Position.Y);
		copyTextLabel->Size = ui::Point(textWidth+20, 16);

		Appearance.TextInactive = ui::Colour(180, 230, 180);
		Appearance.TextHover = ui::Colour(180, 230, 180);
		Appearance.BorderInactive = ui::Colour(180, 230, 180);
		Appearance.BorderHover = ui::Colour(180, 230, 180);
	}

	virtual void OnMouseEnter(int x, int y)
	{
		ui::Button::OnMouseEnter(x, y);
		copyTextLabel->SetTextColour(ui::Colour(230, 230, 230));
	}

	virtual void OnMouseLeave(int x, int y)
	{
		ui::Button::OnMouseLeave(x, y);
		copyTextLabel->SetTextColour(ui::Colour(150, 150, 150));
	}
};

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
	copyTextLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	copyTextLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(copyTextLabel);

	std::stringstream saveID;
	saveID << id;
	textWidth = Graphics::textwidth(saveID.str().c_str());
	CopyTextButton * copyTextButton = new CopyTextButton(ui::Point((Size.X-textWidth-10)/2, 50), ui::Point(textWidth+8, 18), saveID.str(), copyTextLabel);
	copyTextButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	copyTextButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	copyTextButton->Appearance.TextInactive = ui::Colour(150, 150, 150);
	copyTextButton->Appearance.TextActive = ui::Colour(230, 255, 230);
	copyTextButton->Appearance.BorderActive = ui::Colour(230, 255, 230);
	copyTextButton->Appearance.BackgroundActive = style::Colour::InactiveBackground;
	AddComponent(copyTextButton);

	class DismissAction: public ui::ButtonAction
	{
		SaveIDMessage * message;
	public:
		DismissAction(SaveIDMessage * message_) { message = message_; }
		void ActionCallback(ui::Button * sender)
		{
			ui::Engine::Ref().CloseWindow();
			message->SelfDestruct();
		}
	};

	ui::Button * okayButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(Size.X, 16), "OK");
	okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	okayButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	okayButton->SetActionCallback(new DismissAction(this));
	AddComponent(okayButton);
	SetCancelButton(okayButton);
	
	ui::Engine::Ref().ShowWindow(this);
}

void SaveIDMessage::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;

	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 200, 200, 200, 255);
}

void SaveIDMessage::OnTryExit(ExitMethod method)
{
	ui::Engine::Ref().CloseWindow();
	SelfDestruct();
}

SaveIDMessage::~SaveIDMessage()
{

}

