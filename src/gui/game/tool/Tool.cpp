#include "Tool.h"

std::unique_ptr<VideoBuffer> Tool::GetTexture(Vec2<int> size)
{
	return textureGen ? textureGen(ToolID, size) : nullptr;
}

void Tool::Click(Simulation * sim, Brush const &brush, ui::Point position) { }
void Tool::Draw(Simulation * sim, Brush const &brush, ui::Point position) {}
void Tool::DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging) {}
void Tool::DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) {}
void Tool::DrawFill(Simulation * sim, Brush const &brush, ui::Point position) {}
