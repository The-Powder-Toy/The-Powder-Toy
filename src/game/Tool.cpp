/*
 * Tool.cpp
 *
 *  Created on: Jun 24, 2012
 *      Author: Simon
 */

#include <string>
#include "Tool.h"
#include "game/Brush.h"

#include "simulation/Simulation.h"

using namespace std;

Tool::Tool(int id, string name, string description, int r, int g, int b, std::string identifier, VideoBuffer * (*textureGen)(int, int, int)):
	toolID(id),
	toolName(name),
	toolDescription(description),
	colRed(r),
	colGreen(g),
	colBlue(b),
	textureGen(textureGen),
	strength(1.0f),
	resolution(1),
	identifier(identifier)
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
void Tool::SetTextureGen(VideoBuffer * (*textureGen)(int, int, int))
{
	this->textureGen = textureGen;
}
std::string Tool::GetIdentifier() { return identifier; }
string Tool::GetName() { return toolName; }
string Tool::GetDescription() { return toolDescription; }
Tool::~Tool() {}
void Tool::Click(Simulation * sim, Brush * brush, ui::Point position) { }
void Tool::Draw(Simulation * sim, Brush * brush, ui::Point position) {
	sim->ToolBrush(position.X, position.Y, toolID, brush, strength);
}
void Tool::DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging) {
	sim->ToolLine(position1.X, position1.Y, position2.X, position2.Y, toolID, brush, strength);
}
void Tool::DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {
	sim->ToolBox(position1.X, position1.Y, position2.X, position2.Y, toolID, brush, strength);
}
void Tool::DrawFill(Simulation * sim, Brush * brush, ui::Point position) {};

ElementTool::ElementTool(int id, string name, string description, int r, int g, int b, std::string identifier, VideoBuffer * (*textureGen)(int, int, int)):
	Tool(id, name, description, r, g, b, identifier, textureGen)
{
}
ElementTool::~ElementTool() {}
void ElementTool::Draw(Simulation * sim, Brush * brush, ui::Point position){
	sim->CreateParts(position.X, position.Y, toolID, brush);
}
void ElementTool::DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging) {
	sim->CreateLine(position1.X, position1.Y, position2.X, position2.Y, toolID, brush);
}
void ElementTool::DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {
	sim->CreateBox(position1.X, position1.Y, position2.X, position2.Y, toolID, 0);
}
void ElementTool::DrawFill(Simulation * sim, Brush * brush, ui::Point position) {
	sim->FloodParts(position.X, position.Y, toolID, -1, -1, 0);
}


WallTool::WallTool(int id, string name, string description, int r, int g, int b, std::string identifier, VideoBuffer * (*textureGen)(int, int, int)):
Tool(id, name, description, r, g, b, identifier, textureGen)
{
	resolution = CELL;
}
WallTool::~WallTool() {}
void WallTool::Draw(Simulation * sim, Brush * brush, ui::Point position){
	sim->CreateWalls(position.X, position.Y, 1, 1, toolID, 0, brush);
}
void WallTool::DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging) {
	int wallX = position1.X/CELL;
	int wallY = position1.Y/CELL;
	if(dragging == false && toolID == WL_FAN && sim->bmap[wallY][wallX]==WL_FAN)
	{
		float newFanVelX = (position2.X-position1.X)*0.005f;
		newFanVelX *= strength;
		float newFanVelY = (position2.Y-position1.Y)*0.005f;
		newFanVelY *= strength;
		sim->FloodWalls(position1.X, position1.Y, WL_FLOODHELPER, -1, WL_FAN, 0);
		for (int j = 0; j < YRES/CELL; j++)
			for (int i = 0; i < XRES/CELL; i++)
				if (sim->bmap[j][i] == WL_FLOODHELPER)
				{
					sim->fvx[j][i] = newFanVelX;
					sim->fvy[j][i] = newFanVelY;
					sim->bmap[j][i] = WL_FAN;
				}
	}
	else
	{
		sim->CreateWallLine(position1.X, position1.Y, position2.X, position2.Y, 1, 1, toolID, 0, brush);
	}
}
void WallTool::DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {
	sim->CreateWallBox(position1.X, position1.Y, position2.X, position2.Y, toolID, 0);
}
void WallTool::DrawFill(Simulation * sim, Brush * brush, ui::Point position) {
	if (toolID != WL_STREAM)
		sim->FloodWalls(position.X, position.Y, toolID, -1, -1, 0);
}


GolTool::GolTool(int id, string name, string description, int r, int g, int b, std::string identifier, VideoBuffer * (*textureGen)(int, int, int)):
	Tool(id, name, description, r, g, b, identifier, textureGen)
{
}
GolTool::~GolTool() {}
void GolTool::Draw(Simulation * sim, Brush * brush, ui::Point position){
	sim->CreateParts(position.X, position.Y, PT_LIFE|(toolID<<8), brush);
}
void GolTool::DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging) {
	sim->CreateLine(position1.X, position1.Y, position2.X, position2.Y, PT_LIFE|(toolID<<8), brush);
}
void GolTool::DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {
	sim->CreateBox(position1.X, position1.Y, position2.X, position2.Y, PT_LIFE|(toolID<<8), 0);
}
void GolTool::DrawFill(Simulation * sim, Brush * brush, ui::Point position) {
	sim->FloodParts(position.X, position.Y, PT_LIFE|(toolID<<8), -1, -1, 0);
}

WindTool::WindTool(int id, string name, string description, int r, int g, int b, std::string identifier, VideoBuffer * (*textureGen)(int, int, int)):
	Tool(id, name, description, r, g, b, identifier, textureGen)
{
}
WindTool::~WindTool() {}
void WindTool::Draw(Simulation * sim, Brush * brush, ui::Point position)
{

}
void WindTool::DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging)
{
	int radiusX, radiusY, sizeX, sizeY;
	
	float strength = dragging?0.01f:0.002f;
	strength *= this->strength;

	radiusX = brush->GetRadius().X;
	radiusY = brush->GetRadius().Y;
	
	sizeX = brush->GetSize().X;
	sizeY = brush->GetSize().Y;
	
	unsigned char *bitmap = brush->GetBitmap();
	
	for(int y = 0; y < sizeY; y++)
	{
		for(int x = 0; x < sizeX; x++)
		{
			if(bitmap[(y*sizeX)+x] && (position1.X+(x-radiusX) >= 0 && position1.Y+(y-radiusY) >= 0 && position1.X+(x-radiusX) < XRES && position1.Y+(y-radiusY) < YRES))
			{
				sim->vx[(position1.Y+(y-radiusY))/CELL][(position1.X+(x-radiusX))/CELL] += (position2.X-position1.X)*strength;
				sim->vy[(position1.Y+(y-radiusY))/CELL][(position1.X+(x-radiusX))/CELL] += (position2.Y-position1.Y)*strength;
			}
		}
	}
}
void WindTool::DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {}

void WindTool::DrawFill(Simulation * sim, Brush * brush, ui::Point position) {}


void Element_LIGH_Tool::Draw(Simulation * sim, Brush * brush, ui::Point position)
{
	if(sim->currentTick >= nextUse)
	{
		int p = sim->create_part(-2, position.X, position.Y, toolID);
		if (p != -1)
		{
			sim->parts[p].life = brush->GetRadius().X+brush->GetRadius().Y;
			if (sim->parts[p].life > 55)
				sim->parts[p].life = 55;
			sim->parts[p].temp = sim->parts[p].life*150; // temperature of the lighting shows the power of the lighting
			nextUse = sim->currentTick+sim->parts[p].life/4;
		}
	}
}

Element_TESC_Tool::Element_TESC_Tool(int id, string name, string description, int r, int g, int b, std::string identifier, VideoBuffer * (*textureGen)(int, int, int)):
	ElementTool(id, name, description, r, g, b, identifier, textureGen)
	{
	}
void Element_TESC_Tool::Draw(Simulation * sim, Brush * brush, ui::Point position){
	int radiusInfo = brush->GetRadius().X*4+brush->GetRadius().Y*4+7;
	sim->CreateParts(position.X, position.Y, toolID | (radiusInfo << 8), brush);
}
void Element_TESC_Tool::DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging) {
	int radiusInfo = brush->GetRadius().X*4+brush->GetRadius().Y*4+7;
	sim->CreateLine(position1.X, position1.Y, position2.X, position2.Y, toolID | (radiusInfo << 8), brush);
}
void Element_TESC_Tool::DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {
	int radiusInfo = brush->GetRadius().X*4+brush->GetRadius().Y*4+7;
	sim->CreateBox(position1.X, position1.Y, position2.X, position2.Y, toolID | (radiusInfo << 8), 0);
}
void Element_TESC_Tool::DrawFill(Simulation * sim, Brush * brush, ui::Point position) {
	int radiusInfo = brush->GetRadius().X*4+brush->GetRadius().Y*4+7;
	sim->FloodParts(position.X, position.Y, toolID | (radiusInfo << 8), -1, -1, 0);
}

void PlopTool::Click(Simulation * sim, Brush * brush, ui::Point position)
{
	sim->create_part(-1, position.X, position.Y, toolID);
}