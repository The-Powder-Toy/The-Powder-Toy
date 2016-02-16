#ifndef TOOL_H_
#define TOOL_H_

#include <iostream>

using namespace std;

#include "gui/interface/Point.h"
#include "simulation/StructProperty.h"
#include "Lang.h"

class Simulation;
class Brush;
class VideoBuffer;

class Tool
{
protected:
	VideoBuffer * (*textureGen)(int, int, int);
	int toolID;
	string toolName;
	wstring toolDescription;
	float strength;
	int resolution;
	std::string identifier;
public:
	int colRed, colGreen, colBlue;

	Tool(int id, string name, wstring description, int r, int g, int b, std::string identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	int GetToolID() { return toolID; }
	string GetName();
	wstring GetDescription();
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
};

class GameModel;

class SignTool: public Tool
{
public:
	GameModel * gameModel;
	SignTool(GameModel *model):
	Tool(0, "SIGN", TEXT_SPEC_TOOL_SIGN_DESC, 0, 0, 0, "DEFAULT_UI_SIGN", SignTool::GetIcon),
	  gameModel(model)
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

class SampleTool: public Tool
{
	GameModel * gameModel;
public:
	SampleTool(GameModel *model):
	Tool(0, "SMPL", TEXT_SPEC_TOOL_SMPL_DESC, 0, 0, 0, "DEFAULT_UI_SAMPLE", SampleTool::GetIcon),
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
	Tool(0, "PROP", TEXT_SPEC_TOOL_SMPL_PROP, 0xfe, 0xa9, 0x00, "DEFAULT_UI_PROPERTY", NULL)
	{
	}
	StructProperty::PropertyType propType;
	PropertyValue propValue;
	size_t propOffset;

	void OpenWindow(Simulation *sim);
	virtual ~PropertyTool() {}
	virtual void SetProperty(Simulation *sim, ui::Point position);
	virtual void Click(Simulation * sim, Brush * brush, ui::Point position) { }
	virtual void Draw(Simulation *sim, Brush *brush, ui::Point position);
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false);
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2);
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position);
};


class ElementTool: public Tool
{
public:
	ElementTool(int id, string name, wstring description, int r, int g, int b, std::string identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	virtual ~ElementTool();
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position);
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false);
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2);
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position);
};

class Element_LIGH_Tool: public ElementTool
{
public:
	Element_LIGH_Tool(int id, string name, wstring description, int r, int g, int b, std::string identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL):
		ElementTool(id, name, description, r, g, b, identifier, textureGen)
	{ }
	virtual ~Element_LIGH_Tool() { }
	virtual void Click(Simulation * sim, Brush * brush, ui::Point position) { }
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false);
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) { }
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position) { }
};

class Element_TESC_Tool: public ElementTool
{
public:
	Element_TESC_Tool(int id, string name, wstring description, int r, int g, int b, std::string identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL):
		ElementTool(id, name, description, r, g, b, identifier, textureGen)
	{ }
	virtual ~Element_TESC_Tool() {}
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2);
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position);
};

class PlopTool: public ElementTool
{
public:
	PlopTool(int id, string name, wstring description, int r, int g, int b, std::string identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL):
		ElementTool(id, name, description, r, g, b, identifier, textureGen)
	{ }
	virtual ~PlopTool() { }
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position) { }
	virtual void Click(Simulation * sim, Brush * brush, ui::Point position);
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false) { }
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) { }
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position) { }
};

class WallTool: public Tool
{
public:
	WallTool(int id, string name, wstring description, int r, int g, int b, std::string identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	virtual ~WallTool();
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position);
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false);
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2);
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position);
};

class WindTool: public Tool
{
public:
	WindTool(int id, string name, wstring description, int r, int g, int b, std::string identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	virtual ~WindTool() { }
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position) { }
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false);
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) { }
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position) { }
};

#endif /* TOOL_H_ */
