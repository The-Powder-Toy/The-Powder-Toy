#include "ConfirmPrompt.h"
#include "gui/Style.h"
#include "gui/interface/Button.h"
#include "gui/interface/Engine.h"
#include "gui/interface/Label.h"
#include "gui/interface/ScrollPanel.h"
#include "PowderToy.h"

ConfirmPrompt::ConfirmPrompt(std::string title, std::string message, ConfirmDialogueCallback * callback_):
	ui::Window(ui::Point(-1, -1), ui::Point(250, 35)),
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

	class CloseAction: public ui::ButtonAction
	{
	public:
		ConfirmPrompt * prompt;
		DialogueResult result;
		CloseAction(ConfirmPrompt * prompt_, DialogueResult result_) { prompt = prompt_; result = result_; }
		void ActionCallback(ui::Button * sender)
		{
			prompt->CloseActiveWindow();
			if(prompt->callback)
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

	MakeActiveWindow();
}

ConfirmPrompt::ConfirmPrompt(std::string title, std::string message, std::string buttonText, ConfirmDialogueCallback * callback_):
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

	class CloseAction: public ui::ButtonAction
	{
	public:
		ConfirmPrompt * prompt;
		DialogueResult result;
		CloseAction(ConfirmPrompt * prompt_, DialogueResult result_) { prompt = prompt_; result = result_; }
		void ActionCallback(ui::Button * sender)
		{
			prompt->CloseActiveWindow();
			if(prompt->callback)
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

	ui::Button * okayButton = new ui::Button(ui::Point(Size.X-76, Size.Y-16), ui::Point(76, 16), buttonText);
	okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	okayButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	okayButton->Appearance.TextInactive = style::Colour::WarningTitle;
	okayButton->SetActionCallback(new CloseAction(this, ResultOkay));
	AddComponent(okayButton);
	SetOkayButton(okayButton);

	MakeActiveWindow();
}

bool ConfirmPrompt::Blocking(std::string title, std::string message, std::string buttonText)
{
	class BlockingPromptCallback: public ConfirmDialogueCallback {
	public:
		bool & outputResult;
		BlockingPromptCallback(bool & output): outputResult(output) {}
		virtual void ConfirmCallback(ConfirmPrompt::DialogueResult result) {
			if (result == ConfirmPrompt::ResultOkay)
				outputResult = true;
			else
				outputResult = false;
			ui::Engine::Ref().Break();
		}
		virtual ~BlockingPromptCallback() { }
	};
	bool result;
	new ConfirmPrompt(title, message, buttonText, new BlockingPromptCallback(result));
	EngineProcess();
	return result;
}

void ConfirmPrompt::OnDraw()
{
	Graphics * g = GetGraphics();

	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 200, 200, 200, 255);
}

ConfirmPrompt::~ConfirmPrompt() {
	delete callback;
}

