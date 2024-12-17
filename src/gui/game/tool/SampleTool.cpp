#include "SampleTool.h"
#include "PropertyTool.h"
#include "GOLTool.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include "gui/game/GameModel.h"
#include "gui/game/GameView.h"
#include "gui/interface/Colour.h"
#include "simulation/Simulation.h"
#include "simulation/ElementClasses.h"
#include "gui/game/Menu.h"

std::unique_ptr<VideoBuffer> SampleTool::GetIcon(int toolID, Vec2<int> size)
{
	auto texture = std::make_unique<VideoBuffer>(size);
	texture->DrawRect(size.OriginRect(), 0xA0A0A0_rgb);
	texture->BlendChar((size / 2) - Vec2(5, 5), 0xE066, 0xFFFFFF_rgb .WithAlpha(0xFF));
	texture->BlendChar((size / 2) - Vec2(5, 5), 0xE06B, 0x64B4FF_rgb .WithAlpha(0xFF));
	return texture;
}

void SampleTool::Draw(Simulation * sim, Brush const &brush, ui::Point position)
{
	if(gameModel.GetColourSelectorVisibility())
	{
		pixel colour = gameModel.GetView()->GetPixelUnderMouse();
		gameModel.SetColourSelectorColour(RGB::Unpack(colour).WithAlpha(0xFF));
	}
	else
	{
		int i = -1;
		if (sim->photons[position.Y][position.X])
		{
			i = ID(sim->photons[position.Y][position.X]);
		}
		else if (sim->pmap[position.Y][position.X])
		{
			i = ID(sim->pmap[position.Y][position.X]);
		}
		if (i != -1)
		{
			auto *part = &sim->parts[i];
			if (shiftBehaviour)
			{
				auto *propTool = static_cast<PropertyTool *>(gameModel.GetToolFromIdentifier("DEFAULT_UI_PROPERTY"));
				gameModel.SetActiveTool(0, propTool);
				propTool->OpenWindow(gameModel.GetSimulation(), i);
			}
			else if (part->type == PT_LIFE)
			{
				bool found = false;
				for (auto *elementTool : gameModel.GetMenuList()[SC_LIFE]->GetToolList())
				{
					if (elementTool && ID(elementTool->ToolID) == part->ctype)
					{
						gameModel.SetActiveTool(0, elementTool);
						found = true;
						break;
					}
				}
				if (!found)
				{
					static_cast<GOLTool *>(gameModel.GetToolFromIdentifier("DEFAULT_UI_ADDLIFE"))->OpenWindow(gameModel.GetSimulation(), 0, part->ctype, RGB::Unpack(part->dcolour & 0xFFFFFF), RGB::Unpack(part->tmp & 0xFFFFFF));
				}
			}
			else
			{
				auto &sd = SimulationData::Ref();
				auto &elements = sd.elements;
				gameModel.SetActiveTool(0, gameModel.GetToolFromIdentifier(elements[part->type].Identifier));
			}
		}
	}
}
