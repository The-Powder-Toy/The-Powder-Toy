/*
 * ConfirmPrompt.cpp
 *
 *  Created on: Apr 6, 2012
 *      Author: Simon
 */

#include "TextPrompt.h"
#include "interface/Label.h"
#include "interface/Button.h"

class CloseAction: public ui::ButtonAction
{
public:
	TextPrompt * prompt;
	TextPrompt::DialogueResult result;
	CloseAction(TextPrompt * prompt_, TextPrompt::DialogueResult result_) { prompt = prompt_; result = result_; }
	void ActionCallback(ui::Button * sender)
	{
		ui::Engine::Ref().CloseWindow();
		if(prompt->callback)
			prompt->callback->TextCallback(result, prompt->textField->GetText());
		prompt->SelfDestruct(); //TODO: Fix component disposal
	}
};

TextPrompt::TextPrompt(std::string title, std::string message, bool multiline, TextDialogueCallback * callback_):
	ui::Window(ui::Point(-1, -1), ui::Point(200, 75)),
	callback(callback_)
{
	ui::Label * titleLabel = new ui::Label(ui::Point(2, 1), ui::Point(Size.X-4, 16), title);
	titleLabel->SetTextColour(ui::Colour(220, 220, 50));
	titleLabel->SetAlignment(AlignLeft, AlignBottom);
	AddComponent(titleLabel);

	ui::Label * messageLabel = new ui::Label(ui::Point(4, 18), ui::Point(Size.X-8, 60), message);
	messageLabel->SetAlignment(AlignLeft, AlignTop);
	AddComponent(messageLabel);

	ui::Button * cancelButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(Size.X-50, 16), "Cancel");
	cancelButton->SetAlignment(AlignLeft, AlignBottom);
	cancelButton->SetBorderColour(ui::Colour(200, 200, 200));
	cancelButton->SetActionCallback(new CloseAction(this, ResultCancel));
	AddComponent(cancelButton);

	ui::Button * okayButton = new ui::Button(ui::Point(Size.X-50, Size.Y-16), ui::Point(50, 16), "Okay");
	okayButton->SetAlignment(AlignLeft, AlignBottom);
	okayButton->SetTextColour(ui::Colour(220, 220, 50));
	okayButton->SetActionCallback(new CloseAction(this, ResultOkay));
	AddComponent(okayButton);

	textField = new ui::Textbox(ui::Point(4, 32), ui::Point(Size.X-8, 16), "");
	textField->SetAlignment(AlignLeft, AlignBottom);
	AddComponent(textField);

	ui::Engine::Ref().ShowWindow(this);
}

void TextPrompt::OnDraw()
{
	Graphics * g = ui::Engine::Ref().g;

	g->clearrect(Position.X-2, Position.Y-2, Size.X+4, Size.Y+4);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 200, 200, 200, 255);
}

TextPrompt::~TextPrompt() {
	if(callback)
		delete callback;
}

