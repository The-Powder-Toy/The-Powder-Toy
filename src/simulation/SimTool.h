#pragma once
#include "gui/game/tool/Tool.h"
#include "common/String.h"
#include "graphics/Pixel.h"

class Simulation;
struct Particle;
class SimTool : public Tool
{
public:
	int  (*Perform        )(SimTool *tool, Simulation *sim, Particle *cpart, int x, int y, int brushX, int brushY, float strength);
	void (*PerformClick   )(SimTool *tool, Simulation *sim, const Brush &brush, ui::Point position);
	void (*PerformDrag    )(SimTool *tool, Simulation *sim, const Brush &brush, ui::Point position1, ui::Point position2);
	void (*PerformDraw    )(SimTool *tool, Simulation *sim, const Brush &brush, ui::Point position);
	void (*PerformDrawLine)(SimTool *tool, Simulation *sim, const Brush &brush, ui::Point position1, ui::Point position2, bool dragging);
	void (*PerformDrawRect)(SimTool *tool, Simulation *sim, const Brush &brush, ui::Point position1, ui::Point position2);
	void (*PerformDrawFill)(SimTool *tool, Simulation *sim, const Brush &brush, ui::Point position);


#define TOOL_NUMBERS_DECLARE
#include "ToolNumbers.h"
#undef TOOL_NUMBERS_DECLARE

	SimTool();
	void CallPerform(Simulation *sim, ui::Point position, ui::Point brushOffset);

	void Click(Simulation * sim, Brush const &brush, ui::Point position) override;
	void Drag(Simulation *sim, const Brush &brush, ui::Point position1, ui::Point position2) override;
	void Draw(Simulation *sim, const Brush &brush, ui::Point position) override;
	void DrawLine(Simulation *sim, const Brush &brush, ui::Point position1, ui::Point position2, bool dragging) override;
	void DrawRect(Simulation *sim, const Brush &brush, ui::Point position1, ui::Point position2) override;
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override;
};
