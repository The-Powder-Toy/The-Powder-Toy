#pragma once
#include "Component.h"

namespace ui
{

class Spinner: public Component
{
public:
	Spinner(Point position, Point size);
	void Draw(const Point& screenPos) override;
	virtual ~Spinner();
};

}

