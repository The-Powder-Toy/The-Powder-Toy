/*
 * ConfirmPrompt.cpp
 *
 *  Created on: Apr 6, 2012
 *      Author: Simon
 */

#include "ConfirmPrompt.h"
#include "interface/Label.h"
#include "interface/Button.h"

ConfirmPrompt::ConfirmPrompt(std::string title, std::string message, ConfirmDialogueCallback * callback_):
	ui::Window(ui::Point(-1, -1), ui::Point(200, 75)),
	callback(callback_)
{
	ui::Label * titleLabel = new ui::Label(ui::Point(2, 1), ui::Point(Size.X-4, 16), title);
	titleLabel->SetTextColour(ui::Colour(220, 220, 50));
	titleLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	titleLabel->Appearance.VerticalAlign = ui::Appearance::AlignBottom;
	AddComponent(titleLabel);

	ui::Label * messageLabel = new ui::Label(ui::Point(4, 18), ui::Point(Size.X-8, 60), message);
	messageLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	messageLabel->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	AddComponent(messageLabel);

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
			prompt->SelfDestruct(); //TODO: Fix component disposal
		}
	};


	ui::Button * cancelButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(Size.X-50, 16), "Cancel");
	cancelButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	cancelButton->Appearance.VerticalAlign = ui::Appearance::AlignBottom;
	cancelButton->Appearance.BorderInactive = ui::Colour(200, 200, 200);
	cancelButton->SetActionCallback(new CloseAction(this, ResultCancel));
	AddComponent(cancelButton);

	ui::Button * okayButton = new ui::Button(ui::Point(Size.X-50, Size.Y-16), ui::Point(50, 16), "Continue");
	okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	okayButton->Appearance.VerticalAlign = ui::Appearance::AlignBottom;
	okayButton->Appearance.TextInactive = ui::Colour(220, 220, 50);
	okayButton->SetActionCallback(new CloseAction(this, ResultOkay));
	AddComponent(okayButton);

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

