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
	void Tick(float dt) override;
	void Draw(const Point& screenPos) override;
	virtual ~Spinner();
};

}


#endif /* SPINNER_H_ */
