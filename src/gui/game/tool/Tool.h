#pragma once
#include "common/String.h"
#include "common/Vec2.h"
#include "graphics/Pixel.h"
#include "graphics/VideoBuffer.h"
#include "gui/interface/Point.h"
#include "simulation/StructProperty.h"
#include <memory>
#include <optional>

class Simulation;
class Brush;
struct Particle;

class Tool
{
private:
	std::unique_ptr<VideoBuffer> (*textureGen)(int, Vec2<int>) = nullptr;

public:
	int ToolID = 0;
	String Name = "NULL";
	String Description = "NULL Tool, does NOTHING";
	ByteString Identifier = "DEFAULT_TOOL_INVALID";
	RGB<uint8_t> Colour = 0xFFFFFF_rgb;
	bool Blocky = false;
	float Strength = 1.0f;
	bool shiftBehaviour = false;
	bool ctrlBehaviour = false;
	bool altBehaviour = false;

	Tool() = default;

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
	virtual void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging);
	virtual void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2);
	virtual void DrawFill(Simulation * sim, Brush const &brush, ui::Point position);

	virtual void Drag(Simulation *sim, const Brush &brush, ui::Point position1, ui::Point position2)
	{
	}
};


