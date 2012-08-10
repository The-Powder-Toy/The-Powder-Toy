/*
 * ConfirmPrompt.cpp
 *
 *  Created on: Apr 6, 2012
 *      Author: Simon
 */

#include "ConfirmPrompt.h"
#include "Style.h"
#include "interface/Label.h"
#include "interface/Button.h"

ConfirmPrompt::ConfirmPrompt(std::string title, std::string message, ConfirmDialogueCallback * callback_):
	ui::Window(ui::Point(-1, -1), ui::Point(250, 50)),
	callback(callback_)
{
	int width, height;
	ui::Label * titleLabel = new ui::Label(ui::Point(4, 5), ui::Point(Size.X-8, 15), title);
	titleLabel->SetTextColour(style::Colour::WarningTitle);
	titleLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	titleLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(titleLabel);


	ui::Label * messageLabel = new ui::Label(ui::Point(4, 25), ui::Point(Size.X-8, -1), message);
	messageLabel->SetMultiline(true);
	messageLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	messageLabel->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	AddComponent(messageLabel);

	Size.Y += messageLabel->Size.Y+12;
	Position.Y = (ui::Engine::Ref().GetHeight()-Size.Y)/2;

	class CloseAction: public ui::ButtonAction
	{
	public:
		ConfirmPrompt * prompt;
		DialogueResult result;
		CloseAction(ConfirmPrompt * prompt_, DialogueResult result_) { prompt = prompt_; result = result_; }
		void ActionCallback(ui::Button * sender)
		{
			ui::Engine::Ref().CloseWindow();
			prompt->callback->ConfirmCallback(result);
			prompt->SelfDestruct();
		}
	};


	ui::Button * cancelButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(Size.X-75, 16), "Cancel");
	cancelButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	cancelButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	cancelButton->Appearance.BorderInactive = ui::Colour(200, 200, 200);
	cancelButton->SetActionCallback(new CloseAction(this, ResultCancel));
	AddComponent(cancelButton);
	SetCancelButton(cancelButton);

	ui::Button * okayButton = new ui::Button(ui::Point(Size.X-76, Size.Y-16), ui::Point(76, 16), "Continue");
	okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	okayButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	okayButton->Appearance.TextInactive = style::Colour::WarningTitle;
	okayButton->SetActionCallback(new CloseAction(this, ResultOkay));
	AddComponent(okayButton);
	SetOkayButton(okayButton);

	ui::Engine::Ref().ShowWindow(this);
}

void ConfirmPrompt::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;

	g->clearrect(Position.X-2, Position.Y-2, Size.X+4, Size.Y+4);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 200, 200, 200, 255);
}

ConfirmPrompt::~ConfirmPrompt() {
	if(callback)
		delete callback;
}

