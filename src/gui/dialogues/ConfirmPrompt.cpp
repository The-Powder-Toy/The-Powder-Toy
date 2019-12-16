#include "ConfirmPrompt.h"

#include "gui/Style.h"

#include "gui/interface/Button.h"
#include "gui/interface/Engine.h"
#include "gui/interface/Label.h"
#include "gui/interface/ScrollPanel.h"

#include "PowderToy.h"

#include "graphics/Graphics.h"

ConfirmPrompt::ConfirmPrompt(String title, String message, ResultCallback callback_, String buttonText):
	ui::Window(ui::Point(-1, -1), ui::Point(250, 50)),
	callback(callback_)
{
	ui::Label * titleLabel = new ui::Label(ui::Point(4, 5), ui::Point(Size.X-8, 15), title);
	titleLabel->SetTextColour(style::Colour::WarningTitle);
	titleLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	titleLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(titleLabel);


	ui::ScrollPanel *messagePanel = new ui::ScrollPanel(ui::Point(4, 24), ui::Point(Size.X-8, 206));
	AddComponent(messagePanel);

	ui::Label * messageLabel = new ui::Label(ui::Point(4, 0), ui::Point(Size.X-28, -1), message);
	messageLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	messageLabel->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	messageLabel->SetMultiline(true);
	messagePanel->AddChild(messageLabel);

	messagePanel->InnerSize = ui::Point(messagePanel->Size.X, messageLabel->Size.Y+4);

	if (messageLabel->Size.Y < messagePanel->Size.Y)
		messagePanel->Size.Y = messageLabel->Size.Y+4;
	Size.Y += messagePanel->Size.Y+12;
	Position.Y = (ui::Engine::Ref().GetHeight()-Size.Y)/2;

	ui::Button * cancelButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(Size.X-75, 16), "Cancel");
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

	ui::Button * okayButton = new ui::Button(ui::Point(Size.X-76, Size.Y-16), ui::Point(76, 16), buttonText);
	okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	okayButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	okayButton->Appearance.TextInactive = style::Colour::WarningTitle;
	okayButton->SetActionCallback({ [this] {
		CloseActiveWindow();
		if (callback.okay)
			callback.okay();
		SelfDestruct();
	} });
	AddComponent(okayButton);
	SetOkayButton(okayButton);

	MakeActiveWindow();
}

bool ConfirmPrompt::Blocking(String title, String message, String buttonText)
{
	bool outputResult;
	new ConfirmPrompt(title, message, {
		[&outputResult] { outputResult = true; ui::Engine::Ref().Break(); },
		[&outputResult] { outputResult = false; ui::Engine::Ref().Break(); },
	}, buttonText);
	EngineProcess();
	return outputResult;
}

void ConfirmPrompt::OnDraw()
{
	Graphics * g = GetGraphics();

	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 200, 200, 200, 255);
}
