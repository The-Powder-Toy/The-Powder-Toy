#pragma once
#include "gui/interface/Button.h"

class MenuButton : public ui::Button
{
protected:
	// Category glyphs are monochrome alpha masks; state colours (white/black/grey)
	// get tinted by the category's colour, keeping a brightness floor so the
	// darkened "active" state colour doesn't collapse to invisible black.
	ui::Colour ModifyTextColour(ui::Colour c) override
	{
		double brightness = (c.Red + c.Green + c.Blue) / (3.0 * 255.0);
		if (brightness < 0.35)
			brightness = 0.35;
		return ui::Colour(
			uint8_t(categoryColour.Red   * brightness),
			uint8_t(categoryColour.Green * brightness),
			uint8_t(categoryColour.Blue  * brightness),
			c.Alpha
		);
	}

public:
	using ui::Button::Button;
	int menuID;
	bool needsClick;
	ui::Colour categoryColour = ui::Colour(255, 255, 255, 255);
};
