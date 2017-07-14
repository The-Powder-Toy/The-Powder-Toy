#include <iostream>
#include "Slider.h"
#include "Colour.h"
#include "graphics/Graphics.h"

namespace ui {

Slider::Slider(Point position, Point size, int steps):
		Component(position, size),
		sliderSteps(steps),
		sliderPosition(0),
		isMouseDown(false),
		bgGradient(NULL),
		col1(0, 0, 0, 0),
		col2(0, 0, 0, 0)
{

}

void Slider::updatePosition(int position)
{
	if(position < 3)
		position = 3;
	if(position > Size.X-3)
		position = Size.X-3;

	float fPosition = position-3;
	float fSize = Size.X-6;

	float fSliderPosition = (fPosition/fSize)*sliderSteps;//position;//((x-3)/(Size.X-6))*sliderSteps;

	int newSliderPosition = fSliderPosition;

	if(newSliderPosition == sliderPosition)
		return;

	sliderPosition = newSliderPosition;

	if(actionCallback)
	{
		actionCallback->ValueChangedCallback(this);
	}
}

void Slider::OnMouseMoved(int x, int y, int dx, int dy)
{
	if(isMouseDown)
	{
		updatePosition(x);
	}
}

void Slider::OnMouseClick(int x, int y, unsigned button)
{
	isMouseDown = true;
	updatePosition(x);
}

void Slider::OnMouseUp(int x, int y, unsigned button)
{
	if(isMouseDown)
	{
		isMouseDown = false;
	}
}


void Slider::SetColour(Colour col1, Colour col2)
{
	pixel pix[2] = {(pixel)PIXRGB(col1.Red, col1.Green, col1.Blue), (pixel)PIXRGB(col2.Red, col2.Green, col2.Blue)};
	float fl[2] = {0.0f, 1.0f};
	free(bgGradient);
	this->col1 = col1;
	this->col2 = col2;
	bgGradient = (unsigned char*)Graphics::GenerateGradient(pix, fl, 2, Size.X-7);
}

int Slider::GetValue()
{
	return sliderPosition;
}

void Slider::SetValue(int value)
{
	if(value < 0)
		value = 0;
	if(value > sliderSteps)
		value = sliderSteps;
	sliderPosition = value;
}

int Slider::GetSteps()
{
	return sliderSteps;
}

void Slider::SetSteps(int steps)
{
	if(steps < 0)
		steps = 0;
	if(steps < sliderPosition)
		sliderPosition = steps;
	sliderSteps = steps;
}

void Slider::Draw(const Point& screenPos)
{
	Graphics * g = GetGraphics();
	//g->drawrect(screenPos.X, screenPos.Y, Size.X, Size.Y, 255, 255, 255, 255);

	if(bgGradient)
	{
#ifndef OGLI
		for (int j = 3; j < Size.Y-7; j++)
				for (int i = 3; i < Size.X-7; i++)
					g->blendpixel(screenPos.X+i+2, screenPos.Y+j+2, bgGradient[(i-3)*3], bgGradient[(i-3)*3+1], bgGradient[(i-3)*3+2], 255);
#else
		g->gradientrect(screenPos.X+5, screenPos.Y+5, Size.X-10, Size.Y-10, col1.Red, col1.Green, col1.Blue, col1.Alpha, col2.Red, col2.Green, col2.Blue, col2.Alpha);
#endif
	}

	g->drawrect(screenPos.X+3, screenPos.Y+3, Size.X-6, Size.Y-6, 255, 255, 255, 255);

	float fPosition = sliderPosition;
	float fSize = Size.X-6;
	float fSteps = sliderSteps;

	float fSliderX = (fSize/fSteps)*fPosition;//sliderPosition;//((Size.X-6)/sliderSteps)*sliderPosition;
	int sliderX = fSliderX;
	sliderX += 3;

	g->fillrect(screenPos.X+sliderX-2, screenPos.Y+1, 4, Size.Y-2, 20, 20, 20, 255);
	g->drawrect(screenPos.X+sliderX-2, screenPos.Y+1, 4, Size.Y-2, 200, 200, 200, 255);
}

Slider::~Slider()
{
}

} /* namespace ui */
