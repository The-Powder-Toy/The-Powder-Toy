#ifndef SLIDER_H_
#define SLIDER_H_

#include "Component.h"
#include "Colour.h"

namespace ui {
class Slider;
class SliderAction
{
public:
	virtual void ValueChangedCallback(ui::Slider * sender) {}
	virtual ~SliderAction() {}
};
class Slider: public ui::Component {
	friend class SliderAction;
	int sliderSteps;
	int sliderPosition;
	bool isMouseDown;
	unsigned char * bgGradient;
	SliderAction * actionCallback;
	Colour col1, col2;
	void updatePosition(int position);
public:
	Slider(Point position, Point size, int steps);
	virtual void OnMouseMoved(int x, int y, int dx, int dy);
	virtual void OnMouseClick(int x, int y, unsigned button);
	virtual void OnMouseUp(int x, int y, unsigned button);
	virtual void Draw(const Point& screenPos);
	void SetColour(Colour col1, Colour col2);
	void SetActionCallback(SliderAction * action) { actionCallback = action; }
	int GetValue();
	void SetValue(int value);
	int GetSteps();
	void SetSteps(int steps);
	virtual ~Slider();
};

} /* namespace ui */
#endif /* SLIDER_H_ */
