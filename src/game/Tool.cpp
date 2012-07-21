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

Tool::Tool(int id, string name, string description, int r, int g, int b, VideoBuffer * (*textureGen)(int, int, int)):
	toolID(id),
	toolName(name),
	toolDescription(description),
	colRed(r),
	colGreen(g),
	colBlue(b),
	textureGen(textureGen)
{
}
VideoBuffer * Tool::GetTexture(int width, int height)
{
	if(textureGen)
	{
		return textureGen(toolID, width, height);
	}
	return NULL;
}
string Tool::GetName() { return toolName; }
string Tool::GetDescription() { return toolDescription; }
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

ElementTool::ElementTool(int id, string name, string description, int r, int g, int b, VideoBuffer * (*textureGen)(int, int, int)):
	Tool(id, name, description, r, g, b, textureGen)
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


WallTool::WallTool(int id, string name, string description, int r, int g, int b, VideoBuffer * (*textureGen)(int, int, int)):
Tool(id, name, description, r, g, b, textureGen)
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


GolTool::GolTool(int id, string name, string description, int r, int g, int b, VideoBuffer * (*textureGen)(int, int, int)):
	Tool(id, name, description, r, g, b, textureGen)
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

void Element_LIGH_Tool::Draw(Simulation * sim, Brush * brush, ui::Point position)
{
	int p = sim->create_part(-2, position.X, position.Y, toolID);
	if (p != -1)
	{
		sim->parts[p].life = brush->GetRadius().X+brush->GetRadius().Y;
		if (sim->parts[p].life > 55)
			sim->parts[p].life = 55;
		sim->parts[p].temp = sim->parts[p].life*150; // temperature of the lighting shows the power of the lighting
	}
}