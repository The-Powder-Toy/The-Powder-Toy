#include "simulation/ToolCommon.h"
#include "simulation/Air.h"
#include "gui/game/Brush.h"

static void performDrawLine(SimTool *tool, Simulation *sim, const Brush &brush, ui::Point position1, ui::Point position2, bool dragging);
static void performDrag(SimTool *tool, Simulation *sim, const Brush &brush, ui::Point position1, ui::Point position2);

void SimTool::Tool_WIND()
{
	Identifier = "DEFAULT_TOOL_WIND";
	Name = "WIND";
	Colour = 0x404040_rgb;
	Description = "Creates air movement.";
	PerformDrawLine = &performDrawLine;
	PerformDrag = &performDrag;
}

static void performDrawLine(SimTool *tool, Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging)
{
	float strength = dragging?0.01f:0.002f;
	strength *= tool->Strength;

	for (ui::Point off : brush)
	{
		ui::Point coords = position1 + off;
		if (coords.X >= 0 && coords.Y >= 0 && coords.X < XRES && coords.Y < YRES)
		{
			sim->vx[coords.Y / CELL][coords.X / CELL] += (position2 - position1).X * strength;
			sim->vy[coords.Y / CELL][coords.X / CELL] += (position2 - position1).Y * strength;
		}
	}
}

static void performDrag(SimTool *tool, Simulation *sim, const Brush &brush, ui::Point position1, ui::Point position2)
{
	performDrawLine(tool, sim, brush, position1, position2, false);
}
