#include "ElementPopulation.h"
#include "gui/interface/Engine.h"
#include "simulation/Simulation.h"
#include "simulation/SimulationData.h"
#include "graphics/Graphics.h"

ElementPopulationDebug::ElementPopulationDebug(unsigned int id, Simulation * sim):
	DebugInfo(id),
	sim(sim),
	maxAverage(255.0f)
{

}

void ElementPopulationDebug::Draw()
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;
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
		if(elements[i].Enabled)
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


	g->BlendFilledRect(RectSized(Vec2{ xStart-5, yBottom - 263 }, Vec2{ bars+10+Graphics::TextSize(maxValString).X+9, 255 + 13 }), 0x000000_rgb .WithAlpha(180));

	bars = 0;
	for(int i = 0; i < PT_NUM; i++)
	{
		if(elements[i].Enabled)
		{
			auto count = sim->elementCount[i];
			auto barSize = int(count * scale - 0.5f);
			int barX = bars;//*2;

			RGB colour = elements[i].Colour;

			g->DrawLine({ xStart+barX, yBottom+3 }, { xStart+barX, yBottom+2 }, colour);
			if(sim->elementCount[i])
			{
				if(barSize > 256)
				{
					barSize = 256;
					g->DrawPixel({ xStart+barX, yBottom-barSize-3 }, colour);
					g->DrawPixel({ xStart+barX, yBottom-barSize-5 }, colour);
					g->DrawPixel({ xStart+barX, yBottom-barSize-7 }, colour);
				} else {

					g->BlendLine({ xStart+barX, yBottom-barSize-3 }, { xStart+barX, yBottom-barSize-2 }, 0xFFFFFF_rgb .WithAlpha(180));
				}
				g->DrawLine({ xStart+barX, yBottom-barSize }, { xStart+barX, yBottom }, colour);
			}
			bars++;
		}
	}

	g->BlendText({ xStart + bars + 5, yBottom-5 }, "0", 0xFFFFFF_rgb .WithAlpha(255));
	g->BlendText({ xStart + bars + 5, yBottom-132 }, halfValString, 0xFFFFFF_rgb .WithAlpha(255));
	g->BlendText({ xStart + bars + 5, yBottom-260 }, maxValString, 0xFFFFFF_rgb .WithAlpha(255));
}

ElementPopulationDebug::~ElementPopulationDebug()
{

}
