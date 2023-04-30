#include "DebugParts.h"

#include "gui/interface/Engine.h"

#include "simulation/Simulation.h"

#include "graphics/Graphics.h"

DebugParts::DebugParts(unsigned int id, Simulation * sim):
	DebugInfo(id),
	sim(sim)
{

}

void DebugParts::Draw()
{
	Graphics * g = ui::Engine::Ref().g;

	int x = 0, y = 0, lpx = 0, lpy = 0;
	String info = String::Build(sim->parts_lastActiveIndex, "/", NPART, " (", Format::Precision((float)sim->parts_lastActiveIndex/(NPART)*100.0f, 2), "%)");
	for (int i = 0; i < NPART; i++)
	{
		if (sim->parts[i].type)
			g->AddPixel({ x, y }, RGBA<uint8_t>(255, 255, 255, 180));
		else
			g->AddPixel({ x, y }, RGBA<uint8_t>(0, 0, 0, 180));

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
	g->DrawLine({ 0, lpy }, { XRES, lpy }, RGB<uint8_t>(0, 255, 120));
	g->DrawLine({ lpx, 0 }, { lpx, YRES }, RGB<uint8_t>(0, 255, 120));
	g->AddPixel({ lpx, lpy }, RGBA<uint8_t>(255, 50, 50, 220));

	g->AddPixel({ lpx+1, lpy }, RGBA<uint8_t>(255, 50, 50, 120));
	g->AddPixel({ lpx-1, lpy }, RGBA<uint8_t>(255, 50, 50, 120));
	g->AddPixel({ lpx, lpy+1 }, RGBA<uint8_t>(255, 50, 50, 120));
	g->AddPixel({ lpx, lpy-1 }, RGBA<uint8_t>(255, 50, 50, 120));

	g->BlendFilledRect(RectSized(Vec2{ 7, YRES-26}, Vec2{ g->TextSize(info).X + 4, 14}), RGBA<uint8_t>(0, 0, 0, 180));
	g->BlendText({ 10, YRES-22 }, info, RGBA<uint8_t>(255, 255, 255, 255));
}

DebugParts::~DebugParts()
{

}
