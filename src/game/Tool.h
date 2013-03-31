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
	float strength;
	int resolution;
	std::string identifier;
public:
	Tool(int id, string name, string description, int r, int g, int b, std::string identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	int GetToolID() { return toolID; }
	string GetName();
	string GetDescription();
	std::string GetIdentifier();
	int GetResolution() { return resolution; }
	void SetStrength(float value) { strength = value; }
	float  GetStrength() { return strength; }
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
	Tool(0, "SIGN", "Sign. Click a sign to edit or anywhere else to create a new one", 0, 0, 0, "DEFAULT_UI_SIGN", SignTool::GetIcon)
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

class GameModel;

class SampleTool: public Tool
{
	GameModel * gameModel;
public:
	SampleTool(GameModel * model):
	Tool(0, "SMPL", "Sample an element on the screen", 0, 0, 0, "DEFAULT_UI_SAMPLE", SampleTool::GetIcon),
	gameModel(model)
	{
	}
	static VideoBuffer * GetIcon(int toolID, int width, int height);
	virtual ~SampleTool() {}
	virtual void Click(Simulation * sim, Brush * brush, ui::Point position) { }
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position);
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false) { }
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) { }
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position) { }
};

class PropertyTool: public Tool
{
public:
	PropertyTool():
	Tool(0, "PROP", "Property Edit. Click to alter the properties of elements in the field", 0xfe, 0xa9, 0x00, "DEFAULT_UI_PROPERTY", NULL)
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
	Element_LIGH_Tool(int id, string name, string description, int r, int g, int b, std::string identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL):
	Tool(id, name, description, r, g, b, identifier, textureGen),
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
	ElementTool(int id, string name, string description, int r, int g, int b, std::string identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	virtual ~ElementTool();
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position);
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false);
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2);
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position);
};

class Element_TESC_Tool: public ElementTool
{
public:
	Element_TESC_Tool(int id, string name, string description, int r, int g, int b, std::string identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	virtual ~Element_TESC_Tool() {}
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position);
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false);
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2);
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position);
};

class PlopTool: public ElementTool
{
public:
	PlopTool(int id, string name, string description, int r, int g, int b, std::string identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL):
	ElementTool(id, name, description, r, g, b, identifier, textureGen)
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
	WallTool(int id, string name, string description, int r, int g, int b, std::string identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	virtual ~WallTool();
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position);
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false);
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2);
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position);
};

class GolTool: public Tool
{
public:
	GolTool(int id, string name, string description, int r, int g, int b, std::string identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	virtual ~GolTool();
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position);
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false);
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2);
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position);
};

class WindTool: public Tool
{
public:
	WindTool(int id, string name, string description, int r, int g, int b, std::string identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	virtual ~WindTool();
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position);
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false);
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2);
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position);
};

#endif /* TOOL_H_ */
