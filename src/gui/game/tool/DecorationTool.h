#pragma once
#include <memory>
#include "Tool.h"
#include "graphics/Graphics.h"
#include "graphics/RendererFrame.h"

class Renderer;
class GameView;
class DecorationTool: public Tool
{
public:
	RGBA<uint8_t> Colour;
	GameView *gameView;

	std::unique_ptr<VideoBuffer> GetIcon(int toolID, Vec2<int> size);

	DecorationTool(GameView *newGameView, int decoMode, String name, String description, RGB<uint8_t> colour, ByteString identifier):
		Tool(decoMode, name, description, colour, identifier),
		Colour(0x000000_rgb .WithAlpha(0x00)),
		gameView(newGameView)
	{
		MenuSection = SC_DECO;
	}

	void Draw(Simulation * sim, Brush const &brush, ui::Point position) override;
	void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging) override;
	void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) override;
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override;
};
