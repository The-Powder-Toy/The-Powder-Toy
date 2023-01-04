#pragma once
#include "Component.h"

namespace ui
{

class Spinner: public Component
{
	float cValue;
	int tickInternal;
public:
	Spinner(Point position, Point size);
	void Tick(float dt) override;
	void Draw(const Point& screenPos) override;
	virtual ~Spinner();
};

}

