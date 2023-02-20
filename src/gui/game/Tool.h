#pragma once
#include "common/Geometry.h"
#include "common/String.h"
#include "simulation/StructProperty.h"

class Simulation;
class Brush;
class VideoBuffer;

class Tool
{
protected:
	using Pos = Vec2<int>;
	VideoBuffer * (*textureGen)(int, int, int);
	int toolID;
	String toolName;
	String toolDescription;
	float strength;
	bool blocky;
	ByteString identifier;
public:
	int colRed, colGreen, colBlue;

	Tool(int id, String name, String description, int r, int g, int b, ByteString identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	int GetToolID() { return toolID; }
	String GetName();
	String GetDescription();
	ByteString GetIdentifier();
	int GetBlocky() { return blocky; }
	void SetStrength(float value) { strength = value; }
	float  GetStrength() { return strength; }
	VideoBuffer * GetTexture(int width, int height);
	void SetTextureGen(VideoBuffer * (*textureGen)(int, int, int));
	virtual ~Tool();
	virtual void Click(Simulation *, Brush const &, Pos);
	virtual void Draw(Simulation *, Brush const &, Pos);
	virtual void DrawLine(Simulation *, Brush const &, Pos, Pos, bool dragging = false);
	virtual void DrawRect(Simulation *, Brush const &, Pos, Pos);
	virtual void DrawFill(Simulation *, Brush const &, Pos);
};

class GameModel;

class SignTool: public Tool
{
public:
	GameModel * gameModel;
	SignTool(GameModel *model):
	Tool(0, "SIGN", "Sign. Displays text. Click on a sign to edit it or anywhere else to place a new one.", 0, 0, 0, "DEFAULT_UI_SIGN", SignTool::GetIcon),
	  gameModel(model)
	{
	}
	static VideoBuffer * GetIcon(int toolID, int width, int height);
	virtual ~SignTool() {}
	void Click(Simulation *, Brush const &, Pos) override;
	void Draw(Simulation *, Brush const &, Pos) override { }
	void DrawLine(Simulation *, Brush const &, Pos, Pos, bool dragging = false) override { }
	void DrawRect(Simulation *, Brush const &, Pos, Pos) override { }
	void DrawFill(Simulation *, Brush const &, Pos) override { }
};

class SampleTool: public Tool
{
	GameModel * gameModel;
public:
	SampleTool(GameModel *model):
	Tool(0, "SMPL", "Sample an element on the screen.", 0, 0, 0, "DEFAULT_UI_SAMPLE", SampleTool::GetIcon),
	gameModel(model)
	{
	}
	static VideoBuffer * GetIcon(int toolID, int width, int height);
	virtual ~SampleTool() {}
	void Click(Simulation *, Brush const &, Pos) override { }
	void Draw(Simulation *, Brush const &, Pos) override;
	void DrawLine(Simulation *, Brush const &, Pos, Pos, bool dragging = false) override { }
	void DrawRect(Simulation *, Brush const &, Pos, Pos) override { }
	void DrawFill(Simulation *, Brush const &, Pos) override { }
};

class PropertyTool: public Tool
{
public:
	GameModel * gameModel;
	PropertyTool(GameModel *model):
	Tool(0, "PROP", "Property Drawing Tool. Use to alter the properties of elements in the field.", 0xfe, 0xa9, 0x00, "DEFAULT_UI_PROPERTY", NULL),
	gameModel(model)
	{
	}
	StructProperty::PropertyType propType;
	PropertyValue propValue;
	bool changeType;
	size_t propOffset;
	bool validProperty;

	void OpenWindow(Simulation *);
	virtual ~PropertyTool() {}
	virtual void SetProperty(Simulation *, Pos);
	void Click(Simulation *, Brush const &, Pos) override { }
	void Draw(Simulation *, Brush const &brush, Pos) override;
	void DrawLine(Simulation *, Brush const &, Pos, Pos, bool dragging = false) override;
	void DrawRect(Simulation *, Brush const &, Pos, Pos) override;
	void DrawFill(Simulation *, Brush const &, Pos) override;
};

class GOLTool: public Tool
{
public:
	GameModel * gameModel;
	GOLTool(GameModel * gameModel):
	Tool(0, "CUST", "Add a new custom GOL type. (Use ctrl+shift+rightclick to remove them)", 0xfe, 0xa9, 0x00, "DEFAULT_UI_ADDLIFE", NULL),
	gameModel(gameModel)
	{
	}
	void OpenWindow(Simulation *, int toolSelection, int rule = 0, int colour1 = 0, int colour2 = 0);
	virtual ~GOLTool() {}
	void Click(Simulation *, Brush const &, Pos) override { }
	void Draw(Simulation *, Brush const &brush, Pos) override { };
	void DrawLine(Simulation *, Brush const &, Pos, Pos, bool dragging = false) override { };
	void DrawRect(Simulation *, Brush const &, Pos, Pos) override { };
	void DrawFill(Simulation *, Brush const &, Pos) override { };
};


class ElementTool: public Tool
{
public:
	ElementTool(int id, String name, String description, int r, int g, int b, ByteString identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	virtual ~ElementTool();
	void Draw(Simulation *, Brush const &, Pos) override;
	void DrawLine(Simulation *, Brush const &, Pos, Pos, bool dragging = false) override;
	void DrawRect(Simulation *, Brush const &, Pos, Pos) override;
	void DrawFill(Simulation *, Brush const &, Pos) override;
};

class Element_LIGH_Tool: public ElementTool
{
public:
	Element_LIGH_Tool(int id, String name, String description, int r, int g, int b, ByteString identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL):
		ElementTool(id, name, description, r, g, b, identifier, textureGen)
	{ }
	virtual ~Element_LIGH_Tool() { }
	void Click(Simulation *, Brush const &, Pos) override { }
	void DrawLine(Simulation *, Brush const &, Pos, Pos, bool dragging = false) override;
	void DrawRect(Simulation *, Brush const &, Pos, Pos) override { }
	void DrawFill(Simulation *, Brush const &, Pos) override { }
};

class Element_TESC_Tool: public ElementTool
{
public:
	Element_TESC_Tool(int id, String name, String description, int r, int g, int b, ByteString identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL):
		ElementTool(id, name, description, r, g, b, identifier, textureGen)
	{ }
	virtual ~Element_TESC_Tool() {}
	void DrawRect(Simulation *, Brush const &, Pos, Pos) override;
	void DrawFill(Simulation *, Brush const &, Pos) override;
};

class PlopTool: public ElementTool
{
public:
	PlopTool(int id, String name, String description, int r, int g, int b, ByteString identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL):
		ElementTool(id, name, description, r, g, b, identifier, textureGen)
	{ }
	virtual ~PlopTool() { }
	void Draw(Simulation *, Brush const &, Pos) override { }
	void Click(Simulation *, Brush const &, Pos) override;
	void DrawLine(Simulation *, Brush const &, Pos, Pos, bool dragging = false) override { }
	void DrawRect(Simulation *, Brush const &, Pos, Pos) override { }
	void DrawFill(Simulation *, Brush const &, Pos) override { }
};

class WallTool: public Tool
{
public:
	WallTool(int id, String name, String description, int r, int g, int b, ByteString identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	virtual ~WallTool();
	void Draw(Simulation *, Brush const &, Pos) override;
	void DrawLine(Simulation *, Brush const &, Pos, Pos, bool dragging = false) override;
	void DrawRect(Simulation *, Brush const &, Pos, Pos) override;
	void DrawFill(Simulation *, Brush const &, Pos) override;
};

class WindTool: public Tool
{
public:
	WindTool(int id, String name, String description, int r, int g, int b, ByteString identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	virtual ~WindTool() { }
	void Draw(Simulation *, Brush const &, Pos) override { }
	void DrawLine(Simulation *, Brush const &, Pos, Pos, bool dragging = false) override;
	void DrawRect(Simulation *, Brush const &, Pos, Pos) override { }
	void DrawFill(Simulation *, Brush const &, Pos) override { }
};
