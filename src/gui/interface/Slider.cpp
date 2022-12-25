#include "Slider.h"

#include "graphics/Graphics.h"

namespace ui {

Slider::Slider(Point position, Point size, int steps):
		Component(position, size),
		sliderSteps(steps),
		sliderPosition(0),
		isMouseDown(false),
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

	auto fPosition = float(position-3);
	auto fSize = float(Size.X-6);

	float fSliderPosition = (fPosition/fSize)*sliderSteps;//position;//((x-3)/(Size.X-6))*sliderSteps;

	auto newSliderPosition = int(fSliderPosition);

	if(newSliderPosition == sliderPosition)
		return;

	sliderPosition = newSliderPosition;

	if (actionCallback.change)
	{
		actionCallback.change();
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
	this->col1 = col1;
	this->col2 = col2;
	bgGradient = Graphics::Gradient({
		{ pixel(PIXRGB(col1.Red, col1.Green, col1.Blue)), 0.f },
		{ pixel(PIXRGB(col2.Red, col2.Green, col2.Blue)), 1.f },
	}, Size.X-7);
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

	if (bgGradient.size())
	{
		for (int j = 3; j < Size.Y-7; j++)
		{
			for (int i = 3; i < Size.X-7; i++)
			{
				auto color = bgGradient[i - 3];
				g->blendpixel(screenPos.X+i+2, screenPos.Y+j+2, PIXR(color), PIXG(color), PIXB(color), 255);
			}
		}
	}

	g->drawrect(screenPos.X+3, screenPos.Y+3, Size.X-6, Size.Y-6, 255, 255, 255, 255);

	auto fPosition = float(sliderPosition);
	auto fSize = float(Size.X-6);
	auto fSteps = float(sliderSteps);

	auto fSliderX = (fSize/fSteps)*fPosition;//sliderPosition;//((Size.X-6)/sliderSteps)*sliderPosition;
	auto sliderX = int(fSliderX);
	sliderX += 3;

	g->fillrect(screenPos.X+sliderX-2, screenPos.Y+1, 4, Size.Y-2, 20, 20, 20, 255);
	g->drawrect(screenPos.X+sliderX-2, screenPos.Y+1, 4, Size.Y-2, 200, 200, 200, 255);
}

} /* namespace ui */
