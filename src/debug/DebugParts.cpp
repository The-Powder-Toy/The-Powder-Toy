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
			g->AddPixel({ x, y }, 0xFFFFFF_rgb .WithAlpha(180));
		else
			g->AddPixel({ x, y }, 0x000000_rgb .WithAlpha(180));

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
	g->DrawLine({ 0, lpy }, { XRES, lpy }, 0x00FF78_rgb);
	g->DrawLine({ lpx, 0 }, { lpx, YRES }, 0x00FF78_rgb);
	g->AddPixel({ lpx, lpy }, 0xFF3232_rgb .WithAlpha(220));

	g->AddPixel({ lpx+1, lpy }, 0xFF3232_rgb .WithAlpha(120));
	g->AddPixel({ lpx-1, lpy }, 0xFF3232_rgb .WithAlpha(120));
	g->AddPixel({ lpx, lpy+1 }, 0xFF3232_rgb .WithAlpha(120));
	g->AddPixel({ lpx, lpy-1 }, 0xFF3232_rgb .WithAlpha(120));

	g->BlendFilledRect(RectSized(Vec2{ 7, YRES-26}, Vec2{ g->TextSize(info).X + 4, 14}), 0x000000_rgb .WithAlpha(180));
	g->BlendText({ 10, YRES-22 }, info, 0xFFFFFF_rgb .WithAlpha(255));
}

DebugParts::~DebugParts()
{

}
