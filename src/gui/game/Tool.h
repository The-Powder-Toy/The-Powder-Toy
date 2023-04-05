#pragma once
#include <memory>
#include "common/String.h"
#include "common/Vec2.h"
#include "graphics/Pixel.h"
#include "gui/interface/Point.h"
#include "simulation/StructProperty.h"

class Simulation;
class Brush;
class VideoBuffer;

class Tool
{
private:
	std::unique_ptr<VideoBuffer> (*const textureGen)(int, Vec2<int>);

public:
	int const ToolID;
	String const Name;
	String const Description;
	ByteString const Identifier;
	RGB<uint8_t> const Colour;
	bool const Blocky;
	float Strength = 1.0f;

	Tool(int id, String name, String description,
		RGB<uint8_t> colour, ByteString identifier, std::unique_ptr<VideoBuffer> (*textureGen)(int, Vec2<int>) = NULL, bool blocky = false
	):
		textureGen(textureGen),
		ToolID(id),
		Name(name),
		Description(description),
		Identifier(identifier),
		Colour(colour),
		Blocky(blocky)
	{}

	virtual ~Tool()
	{}

	std::unique_ptr<VideoBuffer> GetTexture(Vec2<int>);
	virtual void Click(Simulation * sim, Brush const &brush, ui::Point position);
	virtual void Draw(Simulation * sim, Brush const &brush, ui::Point position);
	virtual void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging = false);
	virtual void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2);
	virtual void DrawFill(Simulation * sim, Brush const &brush, ui::Point position);
};

class GameModel;

class SignTool: public Tool
{
	GameModel &gameModel;

	friend class SignWindow;

public:
	SignTool(GameModel &model):
		Tool(0, "SIGN", "Sign. Displays text. Click on a sign to edit it or anywhere else to place a new one.",
			0x000000_rgb, "DEFAULT_UI_SIGN", SignTool::GetIcon
		),
		gameModel(model)
	{}

	virtual ~SignTool()
	{}

	static std::unique_ptr<VideoBuffer> GetIcon(int toolID, Vec2<int> size);
	void Click(Simulation * sim, Brush const &brush, ui::Point position) override;
	void Draw(Simulation * sim, Brush const &brush, ui::Point position) override { }
	void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging = false) override { }
	void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) override { }
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override { }
};

class SampleTool: public Tool
{
	GameModel &gameModel;

public:
	SampleTool(GameModel &model):
		Tool(0, "SMPL", "Sample an element on the screen.",
			0x000000_rgb, "DEFAULT_UI_SAMPLE", SampleTool::GetIcon
		),
		gameModel(model)
	{}

	static std::unique_ptr<VideoBuffer> GetIcon(int toolID, Vec2<int> size);
	virtual ~SampleTool() {}
	void Click(Simulation * sim, Brush const &brush, ui::Point position) override { }
	void Draw(Simulation * sim, Brush const &brush, ui::Point position) override;
	void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging = false) override { }
	void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) override { }
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override { }
};

class PropertyTool: public Tool
{
	GameModel &gameModel;
	StructProperty::PropertyType propType;
	PropertyValue propValue;
	bool changeType;
	size_t propOffset;
	bool validProperty;

	friend class PropertyWindow;

public:
	PropertyTool(GameModel &model):
		Tool(0, "PROP", "Property Drawing Tool. Use to alter the properties of elements in the field.",
			0xFEA900_rgb, "DEFAULT_UI_PROPERTY", NULL
		),
		gameModel(model)
	{}

	virtual ~PropertyTool()
	{}

	void OpenWindow(Simulation *sim);
	virtual void SetProperty(Simulation *sim, ui::Point position);
	void Click(Simulation * sim, Brush const &brush, ui::Point position) override { }
	void Draw(Simulation *sim, Brush const &brush, ui::Point position) override;
	void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging = false) override;
	void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) override;
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override;
};

class GOLTool: public Tool
{
	GameModel &gameModel;
public:
	GOLTool(GameModel &gameModel):
		Tool(0, "CUST", "Add a new custom GOL type. (Use ctrl+shift+rightclick to remove them)",
			0xFEA900_rgb, "DEFAULT_UI_ADDLIFE", NULL
		),
		gameModel(gameModel)
	{}

	virtual ~GOLTool()
	{}

	void OpenWindow(Simulation *sim, int toolSelection, int rule = 0, RGB<uint8_t> colour1 = 0x000000_rgb, RGB<uint8_t> colour2 = 0x000000_rgb);
	void Click(Simulation * sim, Brush const &brush, ui::Point position) override { }
	void Draw(Simulation *sim, Brush const &brush, ui::Point position) override { };
	void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging = false) override { };
	void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) override { };
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override { };
};


class ElementTool: public Tool
{
public:
	ElementTool(int id, String name, String description,
		RGB<uint8_t> colour, ByteString identifier, std::unique_ptr<VideoBuffer> (*textureGen)(int, Vec2<int>) = NULL):
		Tool(id, name, description, colour, identifier, textureGen)
	{}

	virtual ~ElementTool()
	{}

	void Draw(Simulation * sim, Brush const &brush, ui::Point position) override;
	void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging = false) override;
	void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) override;
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override;
};

class Element_LIGH_Tool: public ElementTool
{
public:
	Element_LIGH_Tool(int id, String name, String description,
		RGB<uint8_t> colour, ByteString identifier, std::unique_ptr<VideoBuffer> (*textureGen)(int, Vec2<int>) = NULL):
		ElementTool(id, name, description, colour, identifier, textureGen)
	{}

	virtual ~Element_LIGH_Tool()
	{}

	void Click(Simulation * sim, Brush const &brush, ui::Point position) override { }
	void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging = false) override;
	void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) override { }
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override { }
};

class Element_TESC_Tool: public ElementTool
{
public:
	Element_TESC_Tool(int id, String name, String description,
		RGB<uint8_t> colour, ByteString identifier, std::unique_ptr<VideoBuffer> (*textureGen)(int, Vec2<int>) = NULL):
		ElementTool(id, name, description, colour, identifier, textureGen)
	{}

	virtual ~Element_TESC_Tool()
	{}

	void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) override;
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override;
};

class PlopTool: public ElementTool
{
public:
	PlopTool(int id, String name, String description,
		RGB<uint8_t> colour, ByteString identifier, std::unique_ptr<VideoBuffer> (*textureGen)(int, Vec2<int>) = NULL):
		ElementTool(id, name, description, colour, identifier, textureGen)
	{}

	virtual ~PlopTool()
	{}

	void Draw(Simulation * sim, Brush const &brush, ui::Point position) override { }
	void Click(Simulation * sim, Brush const &brush, ui::Point position) override;
	void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging = false) override { }
	void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) override { }
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override { }
};

class WallTool: public Tool
{
public:
	WallTool(int id, String description,
		RGB<uint8_t> colour, ByteString identifier, std::unique_ptr<VideoBuffer> (*textureGen)(int, Vec2<int>) = NULL):
		Tool(id, "", description, colour, identifier, textureGen, true)
	{
	}

	virtual ~WallTool()
	{}

	void Draw(Simulation * sim, Brush const &brush, ui::Point position) override;
	void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging = false) override;
	void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) override;
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override;
};

class WindTool: public Tool
{
public:
	WindTool():
		Tool(0, "WIND", "Creates air movement.",
			0x404040_rgb, "DEFAULT_UI_WIND")
	{}

	virtual ~WindTool()
	{}

	void Draw(Simulation * sim, Brush const &brush, ui::Point position) override { }
	void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging = false) override;
	void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) override { }
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override { }
};
