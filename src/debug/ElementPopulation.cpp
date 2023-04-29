#include "ElementPopulation.h"

#include "gui/interface/Engine.h"

#include "simulation/Simulation.h"

#include "graphics/Graphics.h"

ElementPopulationDebug::ElementPopulationDebug(unsigned int id, Simulation * sim):
	DebugInfo(id),
	sim(sim),
	maxAverage(255.0f)
{

}

void ElementPopulationDebug::Draw()
{
	Graphics * g = ui::Engine::Ref().g;

	int yBottom = YRES-10;
	int xStart = 10;

	String maxValString;
	String halfValString;


	float maxVal = 255;
	float scale = 1.0f;
	int bars = 0;
	for(int i = 0; i < PT_NUM; i++)
	{
		if(sim->elements[i].Enabled)
		{
			if(maxVal < sim->elementCount[i])
				maxVal = float(sim->elementCount[i]);
			bars++;
		}
	}
	maxAverage = (maxAverage*(1.0f-0.015f)) + (0.015f*maxVal);
	scale = 255.0f/maxAverage;

	maxValString = String::Build(maxAverage);
	halfValString = String::Build(maxAverage/2);


	g->fillrect(xStart-5, yBottom - 263, bars+10+Graphics::TextSize(maxValString).X+9, 255 + 13, 0, 0, 0, 180);

	bars = 0;
	for(int i = 0; i < PT_NUM; i++)
	{
		if(sim->elements[i].Enabled)
		{
			auto count = sim->elementCount[i];
			auto barSize = int(count * scale - 0.5f);
			int barX = bars;//*2;

			RGB<uint8_t> colour = sim->elements[i].Colour;

			g->draw_line(xStart+barX, yBottom+3, xStart+barX, yBottom+2, colour.Red, colour.Green, colour.Blue, 255);
			if(sim->elementCount[i])
			{
				if(barSize > 256)
				{
					barSize = 256;
					g->blendpixel(xStart+barX, yBottom-barSize-3, colour.Red, colour.Green, colour.Blue, 255);
					g->blendpixel(xStart+barX, yBottom-barSize-5, colour.Red, colour.Green, colour.Blue, 255);
					g->blendpixel(xStart+barX, yBottom-barSize-7, colour.Red, colour.Green, colour.Blue, 255);
				} else {

					g->draw_line(xStart+barX, yBottom-barSize-3, xStart+barX, yBottom-barSize-2, 255, 255, 255, 180);
				}
				g->draw_line(xStart+barX, yBottom-barSize, xStart+barX, yBottom, colour.Red, colour.Green, colour.Blue, 255);
			}
			bars++;
		}
	}

	g->BlendText({ xStart + bars + 5, yBottom-5 }, "0", RGBA<uint8_t>(255, 255, 255, 255));
	g->BlendText({ xStart + bars + 5, yBottom-132 }, halfValString, RGBA<uint8_t>(255, 255, 255, 255));
	g->BlendText({ xStart + bars + 5, yBottom-260 }, maxValString, RGBA<uint8_t>(255, 255, 255, 255));
}

ElementPopulationDebug::~ElementPopulationDebug()
{

}
