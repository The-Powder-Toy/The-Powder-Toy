#include <cmath>
#include <iostream>
#include "Spinner.h"
#include "graphics/Graphics.h"

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
		//g->drawblob(baseX+(sin(cValue+t)*(Size.X/2)), baseY+(cos(cValue+t)*(Size.X/2)), t*255, t*255, t*255);
		g->draw_line(baseX+(sin(cValue+t)*lineInner), baseY+(cos(cValue+t)*lineInner), baseX+(sin(cValue+t)*lineOuter), baseY+(cos(cValue+t)*lineOuter), (t/6)*255, (t/6)*255, (t/6)*255, 255);
	}
}
Spinner::~Spinner()
{

}
