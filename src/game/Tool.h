/*
 * Tool.h
 *
 *  Created on: Jan 22, 2012
 *      Author: Simon
 */

#ifndef TOOL_H_
#define TOOL_H_

#include <iostream>

using namespace std;

class Tool
{
protected:
	int toolID;
	string toolName;
public:
	Tool(int id, string name, int r, int g, int b):
	toolID(id),
	toolName(name),
	colRed(r),
	colGreen(g),
	colBlue(b)
	{
	}
	string GetName() { return toolName; }
	virtual ~Tool() {}
	virtual void Click(Simulation * sim, Brush * brush, ui::Point position) { }
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position) {
		sim->ToolBrush(position.X, position.Y, toolID, brush);
	}
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {
		sim->ToolLine(position1.X, position1.Y, position2.X, position2.Y, toolID, brush);
	}
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {
		sim->ToolBox(position1.X, position1.Y, position2.X, position2.Y, toolID, brush);
	}
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position) {};
	int colRed, colBlue, colGreen;
};

class SignTool: public Tool
{
public:
	SignTool():
	Tool(0, "SIGN", 0, 0, 0)
	{
	}
	virtual ~SignTool() {}
	virtual void Click(Simulation * sim, Brush * brush, ui::Point position);
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position) { }
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) { }
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) { }
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position) { }
};

class PropertyTool: public Tool
{
public:
	PropertyTool():
	Tool(0, "PROP", 0, 0, 0)
	{
	}
	virtual ~PropertyTool() {}
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position) {};
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) { }
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) { }
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position) { }
};

class ElementTool: public Tool
{
public:
	ElementTool(int id, string name, int r, int g, int b):
	Tool(id, name, r, g, b)
	{
	}
	virtual ~ElementTool() {}
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position){
		sim->CreateParts(position.X, position.Y, 1, 1, toolID, 0, brush);
	}
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {
		sim->CreateLine(position1.X, position1.Y, position2.X, position2.Y, 1, 1, toolID, 0, brush);
	}
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {
		sim->CreateBox(position1.X, position1.Y, position2.X, position2.Y, toolID, 0);
	}
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position) {
		sim->FloodParts(position.X, position.Y, toolID, -1, -1, 0);
	}
};

class WallTool: public Tool
{
public:
	WallTool(int id, string name, int r, int g, int b):
	Tool(id, name, r, g, b)
	{
	}
	virtual ~WallTool() {}
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position){
		sim->CreateWalls(position.X, position.Y, 1, 1, toolID, 0, brush);
	}
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {
		sim->CreateWallLine(position1.X, position1.Y, position2.X, position2.Y, 1, 1, toolID, 0, brush);
	}
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {
		sim->CreateWallBox(position1.X, position1.Y, position2.X, position2.Y, toolID, 0);
	}
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position) {
		sim->FloodWalls(position.X, position.Y, toolID, -1, -1, 0);
	}
};

class GolTool: public Tool
{
public:
	GolTool(int id, string name, int r, int g, int b):
		Tool(id, name, r, g, b)
	{
	}
	virtual ~GolTool() {}
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position){
		sim->CreateParts(position.X, position.Y, 1, 1, PT_LIFE|(toolID<<8), 0, brush);
	}
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {
		sim->CreateLine(position1.X, position1.Y, position2.X, position2.Y, 1, 1, PT_LIFE|(toolID<<8), 0, brush);
	}
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {
		sim->CreateBox(position1.X, position1.Y, position2.X, position2.Y, PT_LIFE|(toolID<<8), 0);
	}
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position) {
		sim->FloodParts(position.X, position.Y, PT_LIFE|(toolID<<8), -1, -1, 0);
	}
};

#endif /* TOOL_H_ */
