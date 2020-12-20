#include "ToolButton.h"

#include "graphics/Graphics.h"

#include "gui/interface/Keys.h"
#include "gui/interface/Mouse.h"

#include "Favorite.h"

ToolButton::ToolButton(ui::Point position, ui::Point size, String text, ByteString toolIdentifier, String toolTip):
	ui::Button(position, size, text, toolTip),
	toolIdentifier(toolIdentifier)
{
	SetSelectionState(-1);
	Appearance.BorderActive = ui::Colour(255, 0, 0);
	Appearance.BorderFavorite = ui::Colour(255, 255, 0);

	//don't use "..." on elements that have long names
	buttonDisplayText = ButtonText.Substr(0, 7);
	Component::TextPosition(buttonDisplayText);
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
		if(button == SDL_BUTTON_LEFT)
			SetSelectionState(0);
		if(button == SDL_BUTTON_RIGHT)
			SetSelectionState(1);
		if(button == SDL_BUTTON_MIDDLE)
			SetSelectionState(2);
		DoAction();
	}
}

void ToolButton::OnMouseUp(int x, int y, unsigned int button)
{
	// mouse was unclicked, reset variables in case the unclick happened outside
	isButtonDown = false;
}

void ToolButton::Draw(const ui::Point& screenPos)
{
	Graphics * g = GetGraphics();
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
	if (Favorite::Ref().IsFavorite(toolIdentifier))
	{
		g->drawtext(screenPos.X, screenPos.Y, 0xE068, Appearance.BorderFavorite.Red, Appearance.BorderFavorite.Green, Appearance.BorderFavorite.Blue, Appearance.BorderFavorite.Alpha);
	}

	if (totalColour<544)
	{
		g->drawtext(screenPos.X+textPosition.X, screenPos.Y+textPosition.Y, buttonDisplayText, 255, 255, 255, 255);
	}
	else
	{
		g->drawtext(screenPos.X+textPosition.X, screenPos.Y+textPosition.Y, buttonDisplayText, 0, 0, 0, 255);
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
