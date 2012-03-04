/*
 * Slider.cpp
 *
 *  Created on: Mar 3, 2012
 *      Author: Simon
 */

#include <iostream>
#include "Slider.h"

namespace ui {

Slider::Slider(Point position, Point size, int steps):
		Component(position, size),
		sliderSteps(steps),
		sliderPosition(0),
		isMouseDown(false)
{
	// TODO Auto-generated constructor stub

}

void Slider::updatePosition(int position)
{
	if(position < 3)
		position = 3;
	if(position > Size.X-3)
		position = Size.X-3;

	float fPosition = position-3;
	float fSize = Size.X-6;
	float fSteps = sliderSteps;

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

void Slider::Draw(const Point& screenPos)
{
	Graphics * g = Engine::Ref().g;
	g->drawrect(screenPos.X, screenPos.Y, Size.X, Size.Y, 255, 255, 255, 255);
	g->fillrect(screenPos.X+3, screenPos.Y+3, Size.X-6, Size.Y-6, 255, 255, 255, 255);

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
