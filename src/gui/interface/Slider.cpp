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
		{ col1.NoAlpha(), 0.f },
		{ col2.NoAlpha(), 1.f },
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

	if (bgGradient.size())
	{
		for (int j = 3; j < Size.Y-7; j++)
		{
			for (int i = 3; i < Size.X-7; i++)
			{
				g->DrawPixel(screenPos + Vec2{ i + 2, j + 2 }, bgGradient[i - 3]);
			}
		}
	}

	g->DrawRect(RectSized(screenPos + Vec2{ 3, 3 }, Size - Vec2{ 6, 6 }), 0xFFFFFF_rgb);

	auto fPosition = float(sliderPosition);
	auto fSize = float(Size.X-6);
	auto fSteps = float(sliderSteps);

	auto fSliderX = (fSize/fSteps)*fPosition;//sliderPosition;//((Size.X-6)/sliderSteps)*sliderPosition;
	auto sliderX = int(fSliderX);
	sliderX += 3;

	g->DrawFilledRect(RectSized(screenPos + Vec2{ sliderX-2, 1 }, Vec2{ 4, Size.Y-2 }), 0x141414_rgb);
	g->DrawRect(RectSized(screenPos + Vec2{ sliderX-2, 1 }, Vec2{ 4, Size.Y-2 }), 0xC8C8C8_rgb);
}

} /* namespace ui */
