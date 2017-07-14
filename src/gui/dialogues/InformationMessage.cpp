#include "gui/Style.h"
#include "InformationMessage.h"
#include "gui/interface/Button.h"
#include "gui/interface/Engine.h"
#include "gui/interface/Label.h"
#include "gui/interface/ScrollPanel.h"

InformationMessage::InformationMessage(std::string title, std::string message, bool large):
	ui::Window(ui::Point(-1, -1), ui::Point(200, 35))
{
	if (large) //Maybe also use this large mode for changelogs eventually, or have it as a customizable size?
	{
		Size.X += 200;
		Size.Y += 215;
	}

	if (large)
	{
		ui::ScrollPanel *messagePanel = new ui::ScrollPanel(ui::Point(4, 24), ui::Point(Size.X-8, 206));
		AddComponent(messagePanel);

		ui::Label * messageLabel = new ui::Label(ui::Point(4, 0), ui::Point(Size.X-28, -1), message);
		messageLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
		messageLabel->Appearance.VerticalAlign = ui::Appearance::AlignTop;
		messageLabel->SetMultiline(true);
		messagePanel->AddChild(messageLabel);

		messagePanel->InnerSize = ui::Point(messagePanel->Size.X, messageLabel->Size.Y+4);
	}
	else
	{
		ui::ScrollPanel *messagePanel = new ui::ScrollPanel(ui::Point(4, 24), ui::Point(Size.X-8, 206));
		AddComponent(messagePanel);

		ui::Label * messageLabel = new ui::Label(ui::Point(4, 0), ui::Point(Size.X-8, -1), message);
		messageLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
		messageLabel->Appearance.VerticalAlign = ui::Appearance::AlignTop;
		messageLabel->SetMultiline(true);
		messagePanel->AddChild(messageLabel);

		messagePanel->InnerSize = ui::Point(messagePanel->Size.X, messageLabel->Size.Y+4);

		if (messageLabel->Size.Y < messagePanel->Size.Y)
			messagePanel->Size.Y = messageLabel->Size.Y+4;
		Size.Y += messagePanel->Size.Y+12;
		Position.Y = (ui::Engine::Ref().GetHeight()-Size.Y)/2;
	}

	ui::Label * titleLabel = new ui::Label(ui::Point(4, 5), ui::Point(Size.X-8, 16), title);
	titleLabel->SetTextColour(style::Colour::InformationTitle);
	titleLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	titleLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(titleLabel);

	class DismissAction: public ui::ButtonAction
	{
		InformationMessage * message;
	public:
		DismissAction(InformationMessage * message_) { message = message_; }
		void ActionCallback(ui::Button * sender)
		{
			message->CloseActiveWindow();
			message->SelfDestruct(); //TODO: Fix component disposal
		}
	};

	ui::Button * okayButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(Size.X, 16), "Dismiss");
	okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	okayButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	okayButton->Appearance.BorderInactive = ui::Colour(200, 200, 200);
	okayButton->SetActionCallback(new DismissAction(this));
	AddComponent(okayButton);
	SetOkayButton(okayButton);
	SetCancelButton(okayButton);
	
	MakeActiveWindow();
}

void InformationMessage::OnDraw()
{
	Graphics * g = GetGraphics();

	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 200, 200, 200, 255);
}

InformationMessage::~InformationMessage() {
}

