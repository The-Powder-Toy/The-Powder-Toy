#include <string>
#include "Tool.h"
#include "gui/game/Brush.h"

#include "simulation/Simulation.h"

using namespace std;

Tool::Tool(int id, string name, string description, int r, int g, int b, std::string identifier, VideoBuffer * (*textureGen)(int, int, int)):
	textureGen(textureGen),
	toolID(id),
	toolName(name),
	toolDescription(description),
	strength(1.0f),
	blocky(false),
	identifier(identifier),
	colRed(r),
	colGreen(g),
	colBlue(b)
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
	sim->ToolBox(position1.X, position1.Y, position2.X, position2.Y, toolID, strength);
}
void Tool::DrawFill(Simulation * sim, Brush * brush, ui::Point position) {}


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
	sim->CreateBox(position1.X, position1.Y, position2.X, position2.Y, toolID);
}
void ElementTool::DrawFill(Simulation * sim, Brush * brush, ui::Point position) {
	sim->FloodParts(position.X, position.Y, toolID, -1);
}


WallTool::WallTool(int id, string name, string description, int r, int g, int b, std::string identifier, VideoBuffer * (*textureGen)(int, int, int)):
Tool(id, name, description, r, g, b, identifier, textureGen)
{
	blocky = true;
}
WallTool::~WallTool() {}
void WallTool::Draw(Simulation * sim, Brush * brush, ui::Point position) {
	sim->CreateWalls(position.X, position.Y, 1, 1, toolID, brush);
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
		sim->FloodWalls(position1.X, position1.Y, WL_FLOODHELPER, WL_FAN);
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
		sim->CreateWallLine(position1.X, position1.Y, position2.X, position2.Y, 1, 1, toolID, brush);
	}
}
void WallTool::DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {
	sim->CreateWallBox(position1.X, position1.Y, position2.X, position2.Y, toolID);
}
void WallTool::DrawFill(Simulation * sim, Brush * brush, ui::Point position) {
	if (toolID != WL_STREAM)
		sim->FloodWalls(position.X, position.Y, toolID, -1);
}

WindTool::WindTool(int id, string name, string description, int r, int g, int b, std::string identifier, VideoBuffer * (*textureGen)(int, int, int)):
	Tool(id, name, description, r, g, b, identifier, textureGen)
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


void Element_LIGH_Tool::DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging)
{
	if (dragging)
		sim->CreateParts(position1.X, position1.Y, brush->GetRadius().X, brush->GetRadius().Y, PT_LIGH);
}


void Element_TESC_Tool::DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {
	int radiusInfo = brush->GetRadius().X*4+brush->GetRadius().Y*4+7;
	sim->CreateBox(position1.X, position1.Y, position2.X, position2.Y, toolID | PMAPID(radiusInfo));
}
void Element_TESC_Tool::DrawFill(Simulation * sim, Brush * brush, ui::Point position) {
	int radiusInfo = brush->GetRadius().X*4+brush->GetRadius().Y*4+7;
	sim->FloodParts(position.X, position.Y, toolID | PMAPID(radiusInfo), -1);
}


void PlopTool::Click(Simulation * sim, Brush * brush, ui::Point position)
{
	sim->create_part(-2, position.X, position.Y, TYP(toolID), ID(toolID));
}
