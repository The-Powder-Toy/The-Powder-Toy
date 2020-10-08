#ifndef TOOL_H_
#define TOOL_H_

#include "common/String.h"
#include "gui/interface/Point.h"
#include "simulation/StructProperty.h"

class Simulation;
class Brush;
class VideoBuffer;

class Tool
{
protected:
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
	Tool(0, "SIGN", "Sign. Displays text. Click on a sign to edit it or anywhere else to place a new one.", 0, 0, 0, "DEFAULT_UI_SIGN", SignTool::GetIcon),
	  gameModel(model)
	{
	}
	static VideoBuffer * GetIcon(int toolID, int width, int height);
	virtual ~SignTool() {}
	void Click(Simulation * sim, Brush * brush, ui::Point position) override;
	void Draw(Simulation * sim, Brush * brush, ui::Point position) override { }
	void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false) override { }
	void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) override { }
	void DrawFill(Simulation * sim, Brush * brush, ui::Point position) override { }
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
	void Click(Simulation * sim, Brush * brush, ui::Point position) override { }
	void Draw(Simulation * sim, Brush * brush, ui::Point position) override;
	void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false) override { }
	void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) override { }
	void DrawFill(Simulation * sim, Brush * brush, ui::Point position) override { }
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

	void OpenWindow(Simulation *sim);
	virtual ~PropertyTool() {}
	virtual void SetProperty(Simulation *sim, ui::Point position);
	void Click(Simulation * sim, Brush * brush, ui::Point position) override { }
	void Draw(Simulation *sim, Brush *brush, ui::Point position) override;
	void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false) override;
	void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) override;
	void DrawFill(Simulation * sim, Brush * brush, ui::Point position) override;
};

class GOLTool: public Tool
{
public:
	String selectGOLType;
	GameModel * gameModel;
	GOLTool(GameModel * gameModel):
	Tool(0, "CUST", "Add a new custom GOL type. (Use ctrl+shift+rightclick to remove them)", 0xfe, 0xa9, 0x00, "DEFAULT_UI_ADDLIFE", NULL),
	gameModel(gameModel)
	{
	}
	void OpenWindow(Simulation *sim, int toolSelection, int rule = 0, int colour1 = 0, int colour2 = 0);
	virtual ~GOLTool() {}
	void Click(Simulation * sim, Brush * brush, ui::Point position) override { }
	void Draw(Simulation *sim, Brush *brush, ui::Point position) override { };
	void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false) override { };
	void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) override { };
	void DrawFill(Simulation * sim, Brush * brush, ui::Point position) override { };
};


class ElementTool: public Tool
{
public:
	ElementTool(int id, String name, String description, int r, int g, int b, ByteString identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	virtual ~ElementTool();
	void Draw(Simulation * sim, Brush * brush, ui::Point position) override;
	void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false) override;
	void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) override;
	void DrawFill(Simulation * sim, Brush * brush, ui::Point position) override;
};

class Element_LIGH_Tool: public ElementTool
{
public:
	Element_LIGH_Tool(int id, String name, String description, int r, int g, int b, ByteString identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL):
		ElementTool(id, name, description, r, g, b, identifier, textureGen)
	{ }
	virtual ~Element_LIGH_Tool() { }
	void Click(Simulation * sim, Brush * brush, ui::Point position) override { }
	void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false) override;
	void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) override { }
	void DrawFill(Simulation * sim, Brush * brush, ui::Point position) override { }
};

class Element_TESC_Tool: public ElementTool
{
public:
	Element_TESC_Tool(int id, String name, String description, int r, int g, int b, ByteString identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL):
		ElementTool(id, name, description, r, g, b, identifier, textureGen)
	{ }
	virtual ~Element_TESC_Tool() {}
	void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) override;
	void DrawFill(Simulation * sim, Brush * brush, ui::Point position) override;
};

class PlopTool: public ElementTool
{
public:
	PlopTool(int id, String name, String description, int r, int g, int b, ByteString identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL):
		ElementTool(id, name, description, r, g, b, identifier, textureGen)
	{ }
	virtual ~PlopTool() { }
	void Draw(Simulation * sim, Brush * brush, ui::Point position) override { }
	void Click(Simulation * sim, Brush * brush, ui::Point position) override;
	void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false) override { }
	void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) override { }
	void DrawFill(Simulation * sim, Brush * brush, ui::Point position) override { }
};

class WallTool: public Tool
{
public:
	WallTool(int id, String name, String description, int r, int g, int b, ByteString identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	virtual ~WallTool();
	void Draw(Simulation * sim, Brush * brush, ui::Point position) override;
	void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false) override;
	void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) override;
	void DrawFill(Simulation * sim, Brush * brush, ui::Point position) override;
};

class WindTool: public Tool
{
public:
	WindTool(int id, String name, String description, int r, int g, int b, ByteString identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	virtual ~WindTool() { }
	void Draw(Simulation * sim, Brush * brush, ui::Point position) override { }
	void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false) override;
	void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) override { }
	void DrawFill(Simulation * sim, Brush * brush, ui::Point position) override { }
};

#endif /* TOOL_H_ */
