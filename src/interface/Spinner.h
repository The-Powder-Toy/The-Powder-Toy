#ifndef SPINNER_H_
#define SPINNER_H_

#include "Component.h"

namespace ui
{

class Spinner: public Component
{
	float cValue;
	int tickInternal;
public:
	Spinner(Point position, Point size);
	virtual void Tick(float dt);
	virtual void Draw(const Point& screenPos);
	virtual ~Spinner();
};

}


#endif /* SPINNER_H_ */
