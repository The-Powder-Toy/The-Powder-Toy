#include "ToolButton.h"
#include "graphics/Graphics.h"
#include "graphics/VideoBuffer.h"
#include "Favorite.h"
#include <SDL.h>

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

void ToolButton::OnMouseDown(int x, int y, unsigned int button)
{
	if (MouseDownInside)
	{
		isButtonDown = true;
	}
}

void ToolButton::OnMouseClick(int x, int y, unsigned int button)
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
	auto rect = ClipRect;
	if (ClipRect.Size().X && ClipRect.Size().Y)
		g->SwapClipRect(rect); // old cliprect is now in rect

	int totalColour = Appearance.BackgroundInactive.Blue + (3*Appearance.BackgroundInactive.Green) + (2*Appearance.BackgroundInactive.Red);

	if (Appearance.GetTexture())
	{
		auto *tex = Appearance.GetTexture();
		g->BlendImage(tex->Data(), 255, RectSized(screenPos + Vec2{ 2, 2 }, tex->Size()));
	}
	else
	{
		g->BlendFilledRect(RectSized(screenPos + Vec2{ 2, 2 }, Size - Vec2{ 4, 4 }), Appearance.BackgroundInactive);
	}

	if (isMouseInside && currentSelection == -1)
	{
		g->BlendRect(RectSized(screenPos, Size), Appearance.BorderActive);
	}
	else
	{
		g->BlendRect(RectSized(screenPos, Size), Appearance.BorderInactive);
	}
	if (Favorite::Ref().IsFavorite(toolIdentifier))
	{
		g->BlendText(screenPos, 0xE068, Appearance.BorderFavorite);
	}

	if (totalColour<544)
	{
		g->BlendText(screenPos + textPosition, buttonDisplayText, 0xFFFFFF_rgb .WithAlpha(255));
	}
	else
	{
		g->BlendText(screenPos + textPosition, buttonDisplayText, 0x000000_rgb .WithAlpha(255));
	}

	if (ClipRect.Size().X && ClipRect.Size().Y)
		g->SwapClipRect(rect); // apply old clip rect
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
