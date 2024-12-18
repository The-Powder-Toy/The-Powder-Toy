#include "ElementTool.h"
#include "simulation/Simulation.h"
#include "simulation/ElementCommon.h"
#include "gui/game/Brush.h"

void ElementTool::Draw(Simulation * sim, Brush const &brush, ui::Point position){
	sim->CreateParts(position.X, position.Y, ToolID, brush);
}
void ElementTool::DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging) {
	sim->CreateLine(position1.X, position1.Y, position2.X, position2.Y, ToolID, brush);
}
void ElementTool::DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) {
	sim->CreateBox(position1.X, position1.Y, position2.X, position2.Y, ToolID);
}
void ElementTool::DrawFill(Simulation * sim, Brush const &brush, ui::Point position) {
	sim->FloodParts(position.X, position.Y, ToolID, -1);
}


void Element_LIGH_Tool::DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging)
{
	if (dragging)
		sim->CreateParts(position1.X, position1.Y, brush.GetRadius().X, brush.GetRadius().Y, PT_LIGH);
}


void Element_TESC_Tool::DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) {
	int radiusInfo = brush.GetRadius().X*4+brush.GetRadius().Y*4+7;
	sim->CreateBox(position1.X, position1.Y, position2.X, position2.Y, ToolID | PMAPID(radiusInfo));
}
void Element_TESC_Tool::DrawFill(Simulation * sim, Brush const &brush, ui::Point position) {
	int radiusInfo = brush.GetRadius().X*4+brush.GetRadius().Y*4+7;
	sim->FloodParts(position.X, position.Y, ToolID | PMAPID(radiusInfo), -1);
}


void PlopTool::Click(Simulation * sim, Brush const &brush, ui::Point position)
{
	sim->create_part(-2, position.X, position.Y, TYP(ToolID), ID(ToolID));
}
