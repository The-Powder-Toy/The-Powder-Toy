/*
 * Tool.cpp
 *
 *  Created on: Jun 24, 2012
 *      Author: Simon
 */

#include <string>
#include "Tool.h"

#include "simulation/Simulation.h"

using namespace std;

Tool::Tool(int id, string name, int r, int g, int b):
	toolID(id),
	toolName(name),
	colRed(r),
	colGreen(g),
	colBlue(b)
{
}
string Tool::GetName() { return toolName; }
Tool::~Tool() {}
void Tool::Click(Simulation * sim, Brush * brush, ui::Point position) { }
void Tool::Draw(Simulation * sim, Brush * brush, ui::Point position) {
	sim->ToolBrush(position.X, position.Y, toolID, brush);
}
void Tool::DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {
	sim->ToolLine(position1.X, position1.Y, position2.X, position2.Y, toolID, brush);
}
void Tool::DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {
	sim->ToolBox(position1.X, position1.Y, position2.X, position2.Y, toolID, brush);
}
void Tool::DrawFill(Simulation * sim, Brush * brush, ui::Point position) {};

ElementTool::ElementTool(int id, string name, int r, int g, int b):
	Tool(id, name, r, g, b)
{
}
ElementTool::~ElementTool() {}
void ElementTool::Draw(Simulation * sim, Brush * brush, ui::Point position){
	sim->CreateParts(position.X, position.Y, toolID, brush);
}
void ElementTool::DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {
	sim->CreateLine(position1.X, position1.Y, position2.X, position2.Y, toolID, brush);
}
void ElementTool::DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {
	sim->CreateBox(position1.X, position1.Y, position2.X, position2.Y, toolID, 0);
}
void ElementTool::DrawFill(Simulation * sim, Brush * brush, ui::Point position) {
	sim->FloodParts(position.X, position.Y, toolID, -1, -1, 0);
}


WallTool::WallTool(int id, string name, int r, int g, int b):
Tool(id, name, r, g, b)
{
}
WallTool::~WallTool() {}
void WallTool::Draw(Simulation * sim, Brush * brush, ui::Point position){
	sim->CreateWalls(position.X, position.Y, 1, 1, toolID, 0, brush);
}
void WallTool::DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {
	sim->CreateWallLine(position1.X, position1.Y, position2.X, position2.Y, 1, 1, toolID, 0, brush);
}
void WallTool::DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {
	sim->CreateWallBox(position1.X, position1.Y, position2.X, position2.Y, toolID, 0);
}
void WallTool::DrawFill(Simulation * sim, Brush * brush, ui::Point position) {
	sim->FloodWalls(position.X, position.Y, toolID, -1, -1, 0);
}


GolTool::GolTool(int id, string name, int r, int g, int b):
	Tool(id, name, r, g, b)
{
}
GolTool::~GolTool() {}
void GolTool::Draw(Simulation * sim, Brush * brush, ui::Point position){
	sim->CreateParts(position.X, position.Y, PT_LIFE|(toolID<<8), brush);
}
void GolTool::DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {
	sim->CreateLine(position1.X, position1.Y, position2.X, position2.Y, PT_LIFE|(toolID<<8), brush);
}
void GolTool::DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {
	sim->CreateBox(position1.X, position1.Y, position2.X, position2.Y, PT_LIFE|(toolID<<8), 0);
}
void GolTool::DrawFill(Simulation * sim, Brush * brush, ui::Point position) {
	sim->FloodParts(position.X, position.Y, PT_LIFE|(toolID<<8), -1, -1, 0);
}



