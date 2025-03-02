#include "SimTool.h"
#include "Simulation.h"
#include "gui/game/Brush.h"

void SimTool::CallPerform(Simulation *sim, ui::Point position, ui::Point brushOffset)
{
	Particle * cpart = nullptr;
	int r;
	if ((r = sim->pmap[position.Y][position.X]))
		cpart = &(sim->parts[ID(r)]);
	else if ((r = sim->photons[position.Y][position.X]))
		cpart = &(sim->parts[ID(r)]);
	if (Perform)
	{
		// TODO: maybe do something with the result
		Perform(this, sim, cpart, position.X, position.Y, brushOffset.X, brushOffset.Y, Strength);
	}
}

static void defaultPerformDraw(SimTool *tool, Simulation *sim, const Brush &brush, ui::Point position)
{
	for (ui::Point off : brush)
	{
		ui::Point coords = position + off;
		if (coords.X >= 0 && coords.Y >= 0 && coords.X < XRES && coords.Y < YRES)
		{
			tool->CallPerform(sim, coords, position);
		}
	}
}

static void defaultPerformDrawLine(SimTool *tool, Simulation *sim, const Brush &brush, ui::Point position1, ui::Point position2, bool dragging)
{
	auto x1 = position1.X;
	auto y1 = position1.Y;
	auto x2 = position2.X;
	auto y2 = position2.Y;
	bool reverseXY = abs(y2-y1) > abs(x2-x1);
	int x, y, dx, dy, sy, rx = brush.GetRadius().X, ry = brush.GetRadius().Y;
	float e = 0.0f, de;
	if (reverseXY)
	{
		y = x1;
		x1 = y1;
		y1 = y;
		y = x2;
		x2 = y2;
		y2 = y;
	}
	if (x1 > x2)
	{
		y = x1;
		x1 = x2;
		x2 = y;
		y = y1;
		y1 = y2;
		y2 = y;
	}
	dx = x2 - x1;
	dy = abs(y2 - y1);
	de = dx ? dy/(float)dx : 0.0f;
	y = y1;
	sy = (y1<y2) ? 1 : -1;
	for (x=x1; x<=x2; x++)
	{
		if (reverseXY)
			defaultPerformDraw(tool, sim, brush, { y, x });
		else
			defaultPerformDraw(tool, sim, brush, { x, y });
		e += de;
		if (e >= 0.5f)
		{
			y += sy;
			if (!(rx+ry) && ((y1<y2) ? (y<=y2) : (y>=y2)))
			{
				if (reverseXY)
					defaultPerformDraw(tool, sim, brush, { y, x });
				else
					defaultPerformDraw(tool, sim, brush, { x, y });
			}
			e -= 1.0f;
		}
	}
}

static void defaultPerformDrawRect(SimTool *tool, Simulation *sim, const Brush &brush, ui::Point position1, ui::Point position2)
{
	int brushX = ((position1.X + position2.X) / 2);
	int brushY = ((position1.Y + position2.Y) / 2);
	auto tl = ui::Point{ std::min(position1.X, position2.X), std::min(position1.Y, position2.Y) };
	auto br = ui::Point{ std::max(position1.X, position2.X), std::max(position1.Y, position2.Y) };
	for (auto p : RectBetween(tl, br))
	{
		tool->CallPerform(sim, p, { brushX, brushY });
	}
}

SimTool::SimTool()
{
	Perform = nullptr;
	PerformClick = nullptr;
	PerformDrag = nullptr;
	PerformDraw = defaultPerformDraw;
	PerformDrawLine = defaultPerformDrawLine;
	PerformDrawRect = defaultPerformDrawRect;
	PerformDrawFill = nullptr;
	PerformSelect = nullptr;
}

void SimTool::Click(Simulation * sim, const Brush &brush, ui::Point position)
{
	if (PerformClick)
	{
		PerformClick(this, sim, brush, position);
	}
}

void SimTool::Drag(Simulation *sim, const Brush &brush, ui::Point position1, ui::Point position2)
{
	if (PerformDrag)
	{
		PerformDrag(this, sim, brush, position1, position2);
	}
}

void SimTool::Draw(Simulation *sim, const Brush &brush, ui::Point position)
{
	if (PerformDraw)
	{
		PerformDraw(this, sim, brush, position);
	}
}

void SimTool::DrawLine(Simulation *sim, const Brush &brush, ui::Point position1, ui::Point position2, bool dragging)
{
	if (PerformDrawLine)
	{
		PerformDrawLine(this, sim, brush, position1, position2, dragging);
	}
}

void SimTool::DrawRect(Simulation *sim, const Brush &brush, ui::Point position1, ui::Point position2)
{
	if (PerformDrawRect)
	{
		PerformDrawRect(this, sim, brush, position1, position2);
	}
}

void SimTool::DrawFill(Simulation * sim, const Brush &brush, ui::Point position)
{
	if (PerformDrawFill)
	{
		PerformDrawFill(this, sim, brush, position);
	}
}

void SimTool::Select(int toolSelection)
{
	if (PerformSelect)
	{
		PerformSelect(this, toolSelection);
	}
}
