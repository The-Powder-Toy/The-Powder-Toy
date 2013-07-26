#include "ToolButton.h"
#include "gui/interface/Keys.h"

ToolButton::ToolButton(ui::Point position, ui::Point size, std::string text_, std::string toolTip):
	ui::Button(position, size, text_, toolTip)
{
	SetSelectionState(-1);
	Appearance.BorderActive = ui::Colour(255, 0, 0);
}

void ToolButton::OnMouseClick(int x, int y, unsigned int button)
{
	isButtonDown = true;
}

void ToolButton::OnMouseUnclick(int x, int y, unsigned int button)
{
	if(isButtonDown)
	{
		isButtonDown = false;
		if(button == BUTTON_LEFT)
			SetSelectionState(0);
		if(button == BUTTON_RIGHT)
			SetSelectionState(1);
		if(button == BUTTON_MIDDLE)
			SetSelectionState(2);
		DoAction();
	}
}

void ToolButton::Draw(const ui::Point& screenPos)
{
	Graphics * g = ui::Engine::Ref().g;
	int totalColour = Appearance.BackgroundInactive.Blue + (3*Appearance.BackgroundInactive.Green) + (2*Appearance.BackgroundInactive.Red);

	if (Appearance.GetTexture())
	{
		g->draw_image(Appearance.GetTexture(), screenPos.X+2, screenPos.Y+2, 255);
	}
	else
	{
		g->fillrect(screenPos.X+2, screenPos.Y+2, Size.X-4, Size.Y-4, Appearance.BackgroundInactive.Red, Appearance.BackgroundInactive.Green, Appearance.BackgroundInactive.Blue, Appearance.BackgroundInactive.Alpha);
	}

	if (isMouseInside && currentSelection == -1)
	{
		g->drawrect(screenPos.X, screenPos.Y, Size.X, Size.Y, Appearance.BorderActive.Red, Appearance.BorderActive.Green, Appearance.BorderActive.Blue, Appearance.BorderActive.Alpha);
	}
	else
	{
		g->drawrect(screenPos.X, screenPos.Y, Size.X, Size.Y, Appearance.BorderInactive.Red, Appearance.BorderInactive.Green, Appearance.BorderInactive.Blue, Appearance.BorderInactive.Alpha);
	}

	if (totalColour<544)
	{
		g->drawtext(screenPos.X+textPosition.X, screenPos.Y+textPosition.Y, buttonDisplayText.c_str(), 255, 255, 255, 255);
	}
	else
	{
		g->drawtext(screenPos.X+textPosition.X, screenPos.Y+textPosition.Y, buttonDisplayText.c_str(), 0, 0, 0, 255);
	}
}

void ToolButton::SetSelectionState(int state)
{
	currentSelection = state;
	switch(state)
	{
	case 0:
		Appearance.BorderInactive = ui::Colour(255, 0, 0);
		break;
	case 1:
		Appearance.BorderInactive = ui::Colour(0, 0, 255);
		break;
	case 2:
		Appearance.BorderInactive = ui::Colour(0, 255, 0);
		break;
	case 3:
		Appearance.BorderInactive = ui::Colour(0, 255, 255);
		break;
	default:
		Appearance.BorderInactive = ui::Colour(0, 0, 0);
		break;
	}
}

int ToolButton::GetSelectionState()
{
	return currentSelection;
}

ToolButton::~ToolButton() {
}

