#include "Tool.h"

#include "graphics/Graphics.h"
#include "graphics/Renderer.h"

#include "gui/game/GameModel.h"
#include "gui/interface/Colour.h"

#include "simulation/Simulation.h"
#include "simulation/ElementClasses.h"

#include "Menu.h"

VideoBuffer * SampleTool::GetIcon(int toolID, int width, int height)
{
	VideoBuffer * newTexture = new VideoBuffer(width, height);
	for (int y=0; y<height; y++)
	{
		for (int x=0; x<width; x++)
		{
			pixel pc =  x==0||x==width-1||y==0||y==height-1 ? PIXPACK(0xA0A0A0) : PIXPACK(0x000000);
			newTexture->SetPixel(x, y, PIXR(pc), PIXG(pc), PIXB(pc), 255);
		}
	}
	newTexture->AddCharacter((width/2)-5, (height/2)-5, 0xE066, 255, 255, 255, 255);
	newTexture->BlendPixel(10, 9, 100, 180, 255, 255);
	newTexture->BlendPixel(11, 8, 100, 180, 255, 255);
	newTexture->BlendPixel(12, 7, 100, 180, 255, 255);
	return newTexture;
}

void SampleTool::Draw(Simulation * sim, Brush * brush, ui::Point position)
{
	if(gameModel->GetColourSelectorVisibility())
	{
		pixel colour = gameModel->GetRenderer()->sampleColor;
		gameModel->SetColourSelectorColour(ui::Colour(PIXR(colour), PIXG(colour), PIXB(colour), 255));
	}
	else
	{
		Particle *part = nullptr;
		if (sim->photons[position.Y][position.X])
		{
			part = &sim->parts[ID(sim->photons[position.Y][position.X])];
		}
		else if (sim->pmap[position.Y][position.X])
		{
			part = &sim->parts[ID(sim->pmap[position.Y][position.X])];
		}
		if (part)
		{
			if (part->type == PT_LIFE)
			{
				bool found = false;
				for (auto *elementTool : gameModel->GetMenuList()[SC_LIFE]->GetToolList())
				{
					if (elementTool && ID(elementTool->GetToolID()) == part->ctype)
					{
						gameModel->SetActiveTool(0, elementTool);
						found = true;
						break;
					}
				}
				if (!found)
				{
					((GOLTool *)(gameModel->GetToolFromIdentifier("DEFAULT_UI_ADDLIFE")))->OpenWindow(gameModel->GetSimulation(), 0, part->ctype, part->dcolour, part->tmp);
				}
			}
			else
			{
				Tool * elementTool = gameModel->GetElementTool(part->type);
				if(elementTool)
					gameModel->SetActiveTool(0, elementTool);
			}
		}
	}
}
