#ifndef TOOL_H_
#define TOOL_H_

#include <iostream>

#include "common/String.h"
#include "gui/interface/Point.h"
#include "simulation/StructProperty.h"
#include "simulation/Particle.h"
#include "simulation/Sample.h"

class Simulation;
class GameController;
class Renderer;
class Brush;
class VideoBuffer;

class Tool
{
protected:
	VideoBuffer * (*textureGen)(int, int, int);
	int toolID;
	ByteString toolName;
	String toolDescription;
	float strength;
	bool blocky;
	ByteString identifier;
public:
	int colRed, colGreen, colBlue;

	Tool(int id, ByteString name, String description, int r, int g, int b, ByteString identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	int GetToolID() { return toolID; }
	ByteString GetName();
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
	Tool(0, "SMPL", "Sample an element on the screen.", 0, 0, 0, "DEFAULT_UI_SAMPLE", SampleTool::GetIcon),
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

class ConfigTool: public Tool
{
	class ReleaseTool : public Tool
	{
		ConfigTool * configTool;
	public:
		ReleaseTool(ConfigTool *configTool_):
		Tool(0, "", "", 0, 0, 0, "DEFAULT_UI_CONFIG_RELEASE", NULL),
		configTool(configTool_)
		{
		}
		virtual ~ReleaseTool() {}
		virtual void Click(Simulation * sim, Brush * brush, ui::Point position);
		virtual void Draw(Simulation * sim, Brush * brush, ui::Point position) { }
		virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false) { }
		virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) { }
		virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position) { }
	};

	enum struct ConfigState
	{
		ready,
		drayTmp,
		drayTmp2,
		crayTmp,
		crayTmp2,
		dtecTmp2,
		convTmp,
		ldtcTmp,
		ldtcLife
	};
	GameModel * gameModel;
	int currId;
	Particle configPart;
	int dirx, diry;
	ui::Point cursorPos;
	ConfigState configState;
public:
	ReleaseTool releaseTool;
	ConfigTool(GameModel *model):
	Tool(0, "CNFG", "Configurator.", 0xff, 0xcc, 0, "DEFAULT_UI_CONFIG", NULL),
	gameModel(model),
	cursorPos(0, 0),
	configState(ConfigState::ready),
	releaseTool(ReleaseTool(this))
	{
	}
	virtual ~ConfigTool() {}
	void Reset();
	Particle GetPart();
	static bool IsConfigurableType(int type);
	bool IsConfiguring();
	bool IsConfiguringTemp();
	bool IsConfiguringLife();
	bool IsConfiguringTmp();
	bool IsConfiguringTmp2();
	void CalculatePreview(int x, int y, Simulation *sim);
	void DrawHUD(Renderer *ren);
	void OnSelectFiltTmp(Simulation *sim, int tmp);
	virtual void Click(Simulation * sim, Brush * brush, ui::Point position);
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position) { }
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false) { }
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) { }
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position) { }
private:
	int getIdAt(Simulation *sim, ui::Point position);
	Particle getPartAt(Simulation *sim, ui::Point position);
	bool isSamePart(Particle p1, Particle p2);
	ui::Point projectPoint(Particle part, int sampleX, int sampleY, bool allowDiag = true);
	int getDist(ui::Point relPos, int offset = 0);
	int getDist(Particle part, int sampleX, int sampleY, int offset = 0, bool allowDiag = true);
	void drawRedLine(Renderer *ren, int startx, int starty, int endx, int endy);
	void drawWhiteLine(Renderer *ren, int startx, int starty, int endx, int endy);
	void tripleLine(Renderer *ren, int firstLineLen, int midLineLen, bool drawFirstLine = true, bool drawThirdLine = true);
	void drawDtecBox(Renderer *ren);
};

class PropertyTool: public Tool
{
public:
	PropertyTool():
	Tool(0, "PROP", "Property Drawing Tool. Use to alter the properties of elements in the field.", 0xfe, 0xa9, 0x00, "DEFAULT_UI_PROPERTY", NULL)
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
	ElementTool(int id, ByteString name, String description, int r, int g, int b, ByteString identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	virtual ~ElementTool();
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position);
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false);
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2);
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position);
};

class Element_LIGH_Tool: public ElementTool
{
public:
	Element_LIGH_Tool(int id, ByteString name, String description, int r, int g, int b, ByteString identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL):
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
	Element_TESC_Tool(int id, ByteString name, String description, int r, int g, int b, ByteString identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL):
		ElementTool(id, name, description, r, g, b, identifier, textureGen)
	{ }
	virtual ~Element_TESC_Tool() {}
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2);
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position);
};

class PlopTool: public ElementTool
{
public:
	PlopTool(int id, ByteString name, String description, int r, int g, int b, ByteString identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL):
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
	WallTool(int id, ByteString name, String description, int r, int g, int b, ByteString identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	virtual ~WallTool();
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position);
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false);
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2);
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position);
};

class WindTool: public Tool
{
public:
	WindTool(int id, ByteString name, String description, int r, int g, int b, ByteString identifier, VideoBuffer * (*textureGen)(int, int, int) = NULL);
	virtual ~WindTool() { }
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position) { }
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging = false);
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) { }
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position) { }
};

#endif /* TOOL_H_ */
