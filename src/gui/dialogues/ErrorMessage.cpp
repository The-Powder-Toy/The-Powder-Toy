#include "gui/Style.h"
#include "ErrorMessage.h"
#include "gui/interface/Button.h"
#include "gui/interface/Engine.h"
#include "gui/interface/Label.h"
#include "PowderToy.h"

ErrorMessage::ErrorMessage(std::string title, std::string message,  ErrorMessageCallback * callback_):
	ui::Window(ui::Point(-1, -1), ui::Point(200, 35)),
	callback(callback_)
{
	ui::Label * titleLabel = new ui::Label(ui::Point(4, 5), ui::Point(Size.X-8, 16), title);
	titleLabel->SetTextColour(style::Colour::ErrorTitle);
	titleLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	titleLabel->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	AddComponent(titleLabel);

	ui::Label * messageLabel = new ui::Label(ui::Point(4, 24), ui::Point(Size.X-8, -1), message);
	messageLabel->SetMultiline(true);
	messageLabel->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	messageLabel->Appearance.VerticalAlign = ui::Appearance::AlignTop;
	AddComponent(messageLabel);

	Size.Y += messageLabel->Size.Y+12;
	Position.Y = (ui::Engine::Ref().GetHeight()-Size.Y)/2;

	class DismissAction: public ui::ButtonAction
	{
		ErrorMessage * message;
	public:
		DismissAction(ErrorMessage * message_) { message = message_; }
		void ActionCallback(ui::Button * sender)
		{
			message->CloseActiveWindow();
			if(message->callback)
				message->callback->DismissCallback();
			message->SelfDestruct();
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

void ErrorMessage::Blocking(std::string title, std::string message)
{
	class BlockingDismissCallback: public ErrorMessageCallback {
	public:
		BlockingDismissCallback() {}
		virtual void DismissCallback() {
			ui::Engine::Ref().Break();
		}
		virtual ~BlockingDismissCallback() { }
	};
	new ErrorMessage(title, message, new BlockingDismissCallback());
	EngineProcess();
}

void ErrorMessage::OnDraw()
{
	Graphics * g = GetGraphics();

	g->clearrect(Position.X-2, Position.Y-2, Size.X+3, Size.Y+3);
	g->drawrect(Position.X, Position.Y, Size.X, Size.Y, 200, 200, 200, 255);
}

ErrorMessage::~ErrorMessage() {
	delete callback;
}

