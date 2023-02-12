#include "ErrorMessage.h"

#include "gui/Style.h"

#include "gui/interface/Button.h"
#include "gui/interface/Engine.h"
#include "gui/interface/Label.h"

#include "PowderToySDL.h"

#include "graphics/Graphics.h"

ErrorMessage::ErrorMessage(String title, String message, DismissCallback callback_):
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
	Position.Y = (GetGraphics()->Size().Y - Size.Y)/2;

	ui::Button * okayButton = new ui::Button(ui::Point(0, Size.Y-16), ui::Point(Size.X, 16), "Dismiss");
	okayButton->Appearance.HorizontalAlign = ui::Appearance::AlignLeft;
	okayButton->Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
	okayButton->Appearance.BorderInactive = ui::Colour(200, 200, 200);
	okayButton->SetActionCallback({ [this] {
		CloseActiveWindow();
		if (callback.dismiss)
			callback.dismiss();
		SelfDestruct();
	} });
	AddComponent(okayButton);
	SetOkayButton(okayButton);
	SetCancelButton(okayButton);

	MakeActiveWindow();
}

void ErrorMessage::OnDraw()
{
	Graphics * g = GetGraphics();

	g->DrawFilledRect(RectSized(Position - Vec2{ 1, 1 }, Size + Vec2{ 2, 2 }), 0x000000_rgb);
	g->DrawRect(RectSized(Position, Size), 0xC8C8C8_rgb);
}
