#include <iomanip>
#include <sstream>
#include "DebugParts.h"
#include "gui/interface/Engine.h"
#include "simulation/Simulation.h"

DebugParts::DebugParts(unsigned int id, Simulation * sim):
	DebugInfo(id),
	sim(sim)
{

}

void DebugParts::Draw()
{
	Graphics * g = ui::Engine::Ref().g;

	int x = 0, y = 0, lpx = 0, lpy = 0;
	std::stringstream info;
	info << sim->parts_lastActiveIndex << "/" << NPART << " (" << std::fixed << std::setprecision(2) << (float)sim->parts_lastActiveIndex/(NPART)*100.0f << "%)";
	for (int i = 0; i < NPART; i++)
	{
		if (sim->parts[i].type)
			g->addpixel(x, y, 255, 255, 255, 180);
		else
			g->addpixel(x, y, 0, 0, 0, 180);

		if (i == sim->parts_lastActiveIndex)
		{
			lpx = x;
			lpy = y;
		}
		x++;
		if(x >= XRES)
		{
			y++;
			x = 0;
		}
	}
	g->draw_line(0, lpy, XRES, lpy, 0, 255, 120, 255);
	g->draw_line(lpx, 0, lpx, YRES, 0, 255, 120, 255);
	g->addpixel(lpx, lpy, 255, 50, 50, 220);

	g->addpixel(lpx+1, lpy, 255, 50, 50, 120);
	g->addpixel(lpx-1, lpy, 255, 50, 50, 120);
	g->addpixel(lpx, lpy+1, 255, 50, 50, 120);
	g->addpixel(lpx, lpy-1, 255, 50, 50, 120);

	g->fillrect(7, YRES-26, g->textwidth(info.str().c_str())+5, 14, 0, 0, 0, 180);
	g->drawtext(10, YRES-22, info.str().c_str(), 255, 255, 255, 255);
}

DebugParts::~DebugParts()
{

}
