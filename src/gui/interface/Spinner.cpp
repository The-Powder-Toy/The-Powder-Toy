#include "Spinner.h"
#include "graphics/Graphics.h"
#include "Engine.h"
#include <cmath>

using namespace ui;

Spinner::Spinner(Point position, Point size):
	Component(position, size)
{
}
void Spinner::Draw(const Point& screenPos)
{
	Graphics * g = GetGraphics();
	int baseX = screenPos.X+(Size.X/2);
	int baseY = screenPos.Y+(Size.Y/2);
	int lineInner = (Size.X/2);
	int lineOuter = (Size.X/2)+3;
	auto cValue = std::floor(ui::Engine::Ref().LastTick() * 0.015) * 0.25;
	for(double t = 0.0; t < 6.0; t+=0.25)
	{
		g->DrawLine(
			{ int(baseX+(std::sin(cValue+t)*lineInner)), int(baseY+(std::cos(cValue+t)*lineInner)) },
			{ int(baseX+(std::sin(cValue+t)*lineOuter)), int(baseY+(std::cos(cValue+t)*lineOuter)) },
			RGB(int((t/6)*255), int((t/6)*255), int((t/6)*255)));
	}
}
Spinner::~Spinner()
{

}
