#pragma once
#include "Component.h"
#include "Colour.h"
#include "graphics/Pixel.h"
#include <functional>

namespace ui {
class Slider : public ui::Component
{
	int sliderSteps;
	int sliderPosition;
	bool isMouseDown;
	std::vector<RGB> bgGradient;

	struct SliderAction
	{
		std::function<void ()> change;
	};
	SliderAction actionCallback;

	Colour col1, col2;
	void updatePosition(int position);
public:
	Slider(Point position, Point size, int steps);
	virtual ~Slider() = default;

	void OnMouseMoved(int x, int y) override;
	void OnMouseDown(int x, int y, unsigned button) override;
	void OnMouseUp(int x, int y, unsigned button) override;
	void Draw(const Point& screenPos) override;
	void SetColour(Colour col1, Colour col2);
	inline void SetActionCallback(SliderAction action) { actionCallback = action; }
	int GetValue();
	void SetValue(int value);
	int GetSteps();
	void SetSteps(int steps);
};

} /* namespace ui */
