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

#include "interface/Point.h"

class Simulation;
class Brush;

class Tool
{
protected:
	int toolID;
	string toolName;
	string toolDescription;
public:
	Tool(int id, string name, string description, int r, int g, int b);
	string GetName();
	string GetDescription();
	virtual ~Tool();
	virtual void Click(Simulation * sim, Brush * brush, ui::Point position);
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position);
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2);
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2);
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position);
	int colRed, colBlue, colGreen;
};

class SignTool: public Tool
{
public:
	SignTool():
	Tool(0, "SIGN", "Sign. Click a sign to edit or anywhere else to create a new one", 0, 0, 0)
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
	Tool(0, "PROP", "Property Edit. Click to alter the properties of elements in the field", 0, 0, 0)
	{
	}
	virtual ~PropertyTool() {}
	virtual void Click(Simulation * sim, Brush * brush, ui::Point position);
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position) {};
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) { }
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) { }
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position) { }
};

class Element_LIGH_Tool: public Tool
{
public:
	Element_LIGH_Tool(int id, string name, string description, int r, int g, int b):
	Tool(id, name, description, r, g, b)
	{
	}
	virtual ~Element_LIGH_Tool() {}
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position);
	virtual void Click(Simulation * sim, Brush * brush, ui::Point position) { }
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) { }
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) { }
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position) { }
};

class ElementTool: public Tool
{
public:
	ElementTool(int id, string name, string description, int r, int g, int b);
	virtual ~ElementTool();
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position);
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2);
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2);
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position);
};

class WallTool: public Tool
{
public:
	WallTool(int id, string name, string description, int r, int g, int b);
	virtual ~WallTool();
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position);
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2);
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2);
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position);
};

class GolTool: public Tool
{
public:
	GolTool(int id, string name, string description, int r, int g, int b);
	virtual ~GolTool();
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position);
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2);
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2);
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position);
};

#endif /* TOOL_H_ */
