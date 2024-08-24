#pragma once
#include "Tool.h"

class ElementTool: public Tool
{
public:
	ElementTool(int id, String name, String description,
		RGB<uint8_t> colour, ByteString identifier, std::unique_ptr<VideoBuffer> (*textureGen)(int, Vec2<int>) = NULL):
		Tool(id, name, description, colour, identifier, textureGen)
	{}

	void Draw(Simulation * sim, Brush const &brush, ui::Point position) override;
	void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging) override;
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

	void Click(Simulation * sim, Brush const &brush, ui::Point position) override { }
	void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging) override;
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

	void Draw(Simulation * sim, Brush const &brush, ui::Point position) override { }
	void Click(Simulation * sim, Brush const &brush, ui::Point position) override;
	void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging) override { }
	void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) override { }
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override { }
};
