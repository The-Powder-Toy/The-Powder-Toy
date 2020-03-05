#include "TextPrompt.h"

#include "gui/interface/Label.h"
#include "gui/interface/Button.h"
#include "gui/interface/Engine.h"
#include "gui/interface/Textbox.h"

#include "gui/Style.h"
#include "PowderToy.h"

#include "graphics/Graphics.h"

TextPrompt::TextPrompt(String title, String message, String text, String placeholder, bool multiline, TextDialogueCallback callback_):
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
	cancelButton->SetActionCallback({ [this] {
		CloseActiveWindow();
		if (callback.cancel)
			callback.cancel();
		SelfDestruct();
	} });
	AddComponent(cancelButton);
	SetCancelButton(cancelButton);

	ui::Button * okayButton = new ui::Button(ui::Point(Size.X/2, Size.Y-16), ui::Point(Size.X/2, 16), "Okay");
	okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignRight;
	okayButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	okayButton->Appearance.TextInactive = style::Colour::WarningTitle;
	okayButton->SetActionCallback({ [this] {
		CloseActiveWindow();
		if (callback.text)
			callback.text(textField->GetText());
		SelfDestruct();
	} });
	AddComponent(okayButton);
	SetOkayButton(okayButton);

	MakeActiveWindow();
}

String TextPrompt::Blocking(String title, String message, String text, String placeholder, bool multiline)
{
	String outputString;
	new TextPrompt(title, message, text, placeholder, multiline, { [&outputString](String const &resultText) {
		outputString = resultText;
		ui::Engine::Ref().Break();
	}, [](){
		ui::Engine::Ref().Break();
	}});
	EngineProcess();
	return outputString;
}

void TextPrompt::OnDraw()
{
	Graphics * g = GetGraphics();

	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 200, 200, 200, 255);
}
