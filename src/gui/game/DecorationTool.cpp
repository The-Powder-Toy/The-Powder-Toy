#include "DecorationTool.h"
#include "graphics/Renderer.h"
#include "simulation/SimulationData.h"
#include "simulation/Simulation.h"
#include "GameView.h"

std::unique_ptr<VideoBuffer> DecorationTool::GetIcon(int ToolID, Vec2<int> size)
{
	auto texture = std::make_unique<VideoBuffer>(size);

	if (ToolID == DECO_SMUDGE)
		for (auto pos : size.OriginRect())
			texture->DrawPixel(pos, RGB<uint8_t>(0, 0xFF - 5 * pos.X, 5 * pos.X));
	else if (ToolID == DECO_DRAW || ToolID == DECO_CLEAR)
		texture->BlendFilledRect(size.OriginRect(), Colour);
	else
		texture->DrawFilledRect(size.OriginRect(), 0x323232_rgb);

	if (ToolID == DECO_CLEAR)
	{
		auto reverse = RGB<uint8_t>(Colour.Red + 127, Colour.Green + 127, Colour.Blue + 127).WithAlpha(0xFF);
		texture->BlendChar(size / 2 - Vec2(4, 2), 0xE06C, reverse);
	}
	else
	{
		auto colour = Colour.NoAlpha().WithAlpha(0xFF);
		if (ToolID == DECO_ADD)
			texture->AddChar(Vec2(11, 4), '+', colour);
		else if (ToolID == DECO_SUBTRACT)
			texture->AddChar(Vec2(11, 4), '-', colour);
		else if (ToolID == DECO_MULTIPLY)
			texture->AddChar(Vec2(11, 3), 'x', colour);
		else if (ToolID == DECO_DIVIDE)
			texture->AddChar(Vec2(11, 4), '/', colour);
	}
	return texture;
}

void DecorationTool::Draw(Simulation * sim, Brush const &brush, ui::Point position)
{
	sim->ApplyDecorationPoint(position.X, position.Y, Colour.Red, Colour.Green, Colour.Blue, Colour.Alpha, ToolID, brush);
}

void DecorationTool::DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging)
{
	sim->ApplyDecorationLine(position1.X, position1.Y, position2.X, position2.Y, Colour.Red, Colour.Green, Colour.Blue, Colour.Alpha, ToolID, brush);
}

void DecorationTool::DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2)
{
	sim->ApplyDecorationBox(position1.X, position1.Y, position2.X, position2.Y, Colour.Red, Colour.Green, Colour.Blue, Colour.Alpha, ToolID);
}

void DecorationTool::DrawFill(Simulation * sim, Brush const &brush, ui::Point position)
{
	auto &rendererFrame = gameView->GetRendererFrame();
	if (!rendererFrame.Size().OriginRect().Contains(position))
	{
		return;
	}
	auto loc = RGB<uint8_t>::Unpack(rendererFrame[position]);
	if (ToolID == DECO_CLEAR)
		// TODO: this is actually const-correct
		sim->ApplyDecorationFill(rendererFrame, position.X, position.Y, 0, 0, 0, 0, loc.Red, loc.Green, loc.Blue);
	else
		sim->ApplyDecorationFill(rendererFrame, position.X, position.Y, Colour.Red, Colour.Green, Colour.Blue, Colour.Alpha, loc.Red, loc.Green, loc.Blue);
}

