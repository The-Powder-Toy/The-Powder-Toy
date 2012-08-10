/*
 * InformationMessage.cpp
 *
 *  Created on: Jan 29, 2012
 *      Author: Simon
 */

#include "Style.h"
#include "InformationMessage.h"
#include "interface/Button.h"
#include "interface/Label.h"

InformationMessage::InformationMessage(std::string title, std::string message):
	ui::Window(ui::Point(-1, -1), ui::Point(200, 75))
{
	ui::Label * titleLabel = new ui::Label(ui::Point(4, 5), ui::Point(Size.X-8, 16), title);
	titleLabel->SetTextColour(style::Colour::InformationTitle);
	titleLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	titleLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(titleLabel);

	ui::Label * messageLabel = new ui::Label(ui::Point(4, 24), ui::Point(Size.X-8, 60), message);
	messageLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	messageLabel->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	AddComponent(messageLabel);

	class DismissAction: public ui::ButtonAction
	{
		InformationMessage * message;
	public:
		DismissAction(InformationMessage * message_) { message = message_; }
		void ActionCallback(ui::Button * sender)
		{
			ui::Engine::Ref().CloseWindow();
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
	
	ui::Engine::Ref().ShowWindow(this);
}

void InformationMessage::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;

	g->clearrect(Position.X-2, Position.Y-2, Size.X+4, Size.Y+4);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 200, 200, 200, 255);
}

InformationMessage::~InformationMessage() {
}

