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
class VideoBuffer;

class Tool
{
protected:
	VideoBuffer * (*textureGen)(int, int, int);
	int toolID;
	string toolName;
	string toolDescription;
public:
	Tool(int id, string name, string description, int r, int g, int b, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	string GetName();
	string GetDescription();
	VideoBuffer * GetTexture(int width, int height);
	void SetTextureGen(VideoBuffer * (*textureGen)(int, int, int));
	virtual ~Tool();
	virtual void Click(Simulation * sim, Brush * brush, ui::Point position);
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position);
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false);
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2);
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position);
	int colRed, colBlue, colGreen;
};

class SignTool: public Tool
{
public:
	SignTool():
	Tool(0, "SIGN", "Sign. Click a sign to edit or anywhere else to create a new one", 0, 0, 0, SignTool::GetIcon)
	{
	}
	static VideoBuffer * GetIcon(int toolID, int width, int height);
	virtual ~SignTool() {}
	virtual void Click(Simulation * sim, Brush * brush, ui::Point position);
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position) { }
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false) { }
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) { }
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position) { }
};

class PropertyTool: public Tool
{
public:
	PropertyTool():
	Tool(0, "PROP", "Property Edit. Click to alter the properties of elements in the field", 0xfe, 0xa9, 0x00, NULL)
	{
	}
	virtual ~PropertyTool() {}
	virtual void Click(Simulation * sim, Brush * brush, ui::Point position);
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position) {};
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false) { }
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) { }
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position) { }
};

class Element_LIGH_Tool: public Tool
{
	int nextUse;
public:
	Element_LIGH_Tool(int id, string name, string description, int r, int g, int b, VideoBuffer * (*textureGen)(int, int, int) = NULL):
	Tool(id, name, description, r, g, b),
	nextUse(0)
	{
	}
	virtual ~Element_LIGH_Tool() {}
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position);
	virtual void Click(Simulation * sim, Brush * brush, ui::Point position) { }
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false) { }
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) { }
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position) { }
};


class ElementTool: public Tool
{
public:
	ElementTool(int id, string name, string description, int r, int g, int b, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	virtual ~ElementTool();
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position);
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false);
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2);
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position);
};

class Element_TESC_Tool: public ElementTool
{
public:
	Element_TESC_Tool(int id, string name, string description, int r, int g, int b, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	virtual ~Element_TESC_Tool() {}
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position);
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false);
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2);
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position);
};

class PlopTool: public ElementTool
{
public:
	PlopTool(int id, string name, string description, int r, int g, int b, VideoBuffer * (*textureGen)(int, int, int) = NULL):
	ElementTool(id, name, description, r, g, b)
	{
	}
	virtual ~PlopTool() {}
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position) {}
	virtual void Click(Simulation * sim, Brush * brush, ui::Point position);
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false) { }
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) { }
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position) { }
};

class WallTool: public Tool
{
public:
	WallTool(int id, string name, string description, int r, int g, int b, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	virtual ~WallTool();
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position);
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false);
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2);
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position);
};

class GolTool: public Tool
{
public:
	GolTool(int id, string name, string description, int r, int g, int b, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	virtual ~GolTool();
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position);
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false);
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2);
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position);
};

class WindTool: public Tool
{
public:
	WindTool(int id, string name, string description, int r, int g, int b, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	virtual ~WindTool();
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position);
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false);
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2);
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position);
};

#endif /* TOOL_H_ */
