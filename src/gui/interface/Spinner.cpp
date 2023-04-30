#include "Spinner.h"
#include "graphics/Graphics.h"
#include <cmath>

using namespace ui;

Spinner::Spinner(Point position, Point size):
	Component(position, size), cValue(0),
	tickInternal(0)
{
}
void Spinner::Tick(float dt)
{
	tickInternal++;
	if(tickInternal == 4)
	{
		cValue += 0.25f;//0.05f;
		tickInternal = 0;
	}
}
void Spinner::Draw(const Point& screenPos)
{
	Graphics * g = GetGraphics();
	int baseX = screenPos.X+(Size.X/2);
	int baseY = screenPos.Y+(Size.Y/2);
	int lineInner = (Size.X/2);
	int lineOuter = (Size.X/2)+3;
	for(float t = 0.0f; t < 6.0f; t+=0.25f)
	{
		g->DrawLine(
			{ int(baseX+(sin(cValue+t)*lineInner)), int(baseY+(cos(cValue+t)*lineInner)) },
			{ int(baseX+(sin(cValue+t)*lineOuter)), int(baseY+(cos(cValue+t)*lineOuter)) },
			RGB<uint8_t>(int((t/6)*255), int((t/6)*255), int((t/6)*255)));
	}
}
Spinner::~Spinner()
{

}
