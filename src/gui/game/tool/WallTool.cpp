#include "WallTool.h"
#include "simulation/Simulation.h"
#include "simulation/SimulationData.h"

void WallTool::Draw(Simulation * sim, Brush const &brush, ui::Point position) {
	sim->CreateWalls(position.X, position.Y, 1, 1, ToolID, &brush);
}
void WallTool::DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging) {
	int wallX = position1.X/CELL;
	int wallY = position1.Y/CELL;
	if(dragging == false && ToolID == WL_FAN && sim->bmap[wallY][wallX]==WL_FAN)
	{
		float newFanVelX = (position2.X-position1.X)*0.005f;
		newFanVelX *= Strength;
		float newFanVelY = (position2.Y-position1.Y)*0.005f;
		newFanVelY *= Strength;
		sim->FloodWalls(position1.X, position1.Y, WL_FLOODHELPER, WL_FAN);
		for (int j = 0; j < YCELLS; j++)
			for (int i = 0; i < XCELLS; i++)
				if (sim->bmap[j][i] == WL_FLOODHELPER)
				{
					sim->fvx[j][i] = newFanVelX;
					sim->fvy[j][i] = newFanVelY;
					sim->bmap[j][i] = WL_FAN;
				}
	}
	else
	{
		sim->CreateWallLine(position1.X, position1.Y, position2.X, position2.Y, 1, 1, ToolID, &brush);
	}
}
void WallTool::DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) {
	sim->CreateWallBox(position1.X, position1.Y, position2.X, position2.Y, ToolID);
}
void WallTool::DrawFill(Simulation * sim, Brush const &brush, ui::Point position) {
	if (ToolID != WL_STREAM)
		sim->FloodWalls(position.X, position.Y, ToolID, -1);
}
