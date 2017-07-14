#include <iostream>
#include "TextPrompt.h"
#include "gui/interface/Label.h"
#include "gui/interface/Button.h"
#include "gui/interface/Engine.h"
#include "gui/Style.h"
#include "PowderToy.h"

class CloseAction: public ui::ButtonAction
{
public:
	TextPrompt * prompt;
	TextPrompt::DialogueResult result;
	CloseAction(TextPrompt * prompt_, TextPrompt::DialogueResult result_) { prompt = prompt_; result = result_; }
	void ActionCallback(ui::Button * sender)
	{
		prompt->CloseActiveWindow();
		if(prompt->callback)
			prompt->callback->TextCallback(result, prompt->textField->GetText());
		prompt->SelfDestruct(); //TODO: Fix component disposal
	}
};

TextPrompt::TextPrompt(std::string title, std::string message, std::string text, std::string placeholder, bool multiline, TextDialogueCallback * callback_):
	ui::Window(ui::Point(-1, -1), ui::Point(200, 65)),
	callback(callback_)
{
	if(multiline)
		Size.X += 100;

	ui::Label * titleLabel = new ui::Label(ui::Point(4, 5), ui::Point(Size.X-8, 18), title);
	titleLabel->SetTextColour(style::Colour::WarningTitle);
	titleLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	titleLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(titleLabel);

	ui::Label * messageLabel = new ui::Label(ui::Point(4, 25), ui::Point(Size.X-8, -1), message);
	messageLabel->SetMultiline(true);
	messageLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	messageLabel->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	AddComponent(messageLabel);

	Size.Y += messageLabel->Size.Y+4;

	textField = new ui::Textbox(ui::Point(4, messageLabel->Position.Y + messageLabel->Size.Y + 7), ui::Point(Size.X-8, 16), text, placeholder);
	if(multiline)
	{
		textField->SetMultiline(true);
		textField->Size.Y = 60;
		Size.Y += 45;
		textField->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	}
	else
	{
		textField->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	}
	textField->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	AddComponent(textField);
	FocusComponent(textField);

	ui::Button * cancelButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point((Size.X/2)+1, 16), "Cancel");
	cancelButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	cancelButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	cancelButton->Appearance.BorderInactive = ui::Colour(200, 200, 200);
	cancelButton->SetActionCallback(new CloseAction(this, ResultCancel));
	AddComponent(cancelButton);
	SetCancelButton(cancelButton);

	ui::Button * okayButton = new ui::Button(ui::Point(Size.X/2, Size.Y-16), ui::Point(Size.X/2, 16), "Okay");
	okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignRight;
	okayButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	okayButton->Appearance.TextInactive = style::Colour::WarningTitle;
	okayButton->SetActionCallback(new CloseAction(this, ResultOkay));
	AddComponent(okayButton);
	SetOkayButton(okayButton);

	MakeActiveWindow();
}

std::string TextPrompt::Blocking(std::string title, std::string message, std::string text, std::string placeholder, bool multiline)
{
	std::string returnString = "";

	class BlockingTextCallback: public TextDialogueCallback {
		std::string & outputString;
	public:
		BlockingTextCallback(std::string & output) : outputString(output) {}
		virtual void TextCallback(TextPrompt::DialogueResult result, std::string resultText) {
			if(result == ResultOkay)
				outputString = resultText;
			else
				outputString = "";
			ui::Engine::Ref().Break();
		}
		virtual ~BlockingTextCallback() { }
	};
	new TextPrompt(title, message, text, placeholder, multiline, new BlockingTextCallback(returnString));
	EngineProcess();

	return returnString;
}

void TextPrompt::OnDraw()
{
	Graphics * g = GetGraphics();

	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 200, 200, 200, 255);
}

TextPrompt::~TextPrompt() {
	delete callback;
}

