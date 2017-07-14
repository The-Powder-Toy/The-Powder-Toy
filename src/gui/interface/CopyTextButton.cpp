#include "CopyTextButton.h"
#include "Colour.h"
#include "PowderToy.h"
#include "gui/Style.h"

namespace ui
{
	CopyTextButton::CopyTextButton(Point position, Point size, std::string buttonText, Label *copyTextLabel_):
		Button(position, size, buttonText)
	{
		copyTextLabel = copyTextLabel_;
		Appearance.HorizontalAlign = ui::Appearance::AlignCentre;
		Appearance.VerticalAlign = ui::Appearance::AlignMiddle;
		Appearance.TextInactive = ui::Colour(150, 150, 150);
		Appearance.TextActive = ui::Colour(230, 255, 230);
		Appearance.BorderActive = ui::Colour(230, 255, 230);
		Appearance.BackgroundActive = style::Colour::InactiveBackground;
	}

	void CopyTextButton::OnMouseClick(int x, int y, unsigned int button)
	{
		ui::Button::OnMouseClick(x, y, button);
		ClipboardPush(ButtonText);

		copyTextLabel->SetText("Copied!");

		Appearance.TextInactive = ui::Colour(180, 230, 180);
		Appearance.TextHover = ui::Colour(180, 230, 180);
		Appearance.BorderInactive = ui::Colour(180, 230, 180);
		Appearance.BorderHover = ui::Colour(180, 230, 180);
	}

	void CopyTextButton::OnMouseEnter(int x, int y)
	{
		ui::Button::OnMouseEnter(x, y);
		copyTextLabel->SetTextColour(ui::Colour(230, 230, 230));
	}

	void CopyTextButton::OnMouseLeave(int x, int y)
	{
		ui::Button::OnMouseLeave(x, y);
		copyTextLabel->SetTextColour(ui::Colour(150, 150, 150));
	}
} /* namespace ui */
