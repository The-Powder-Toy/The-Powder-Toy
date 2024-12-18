#pragma once
#include "Tool.h"

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

	static std::unique_ptr<VideoBuffer> GetIcon(int toolID, Vec2<int> size);
	void Click(Simulation * sim, Brush const &brush, ui::Point position) override;
	void Draw(Simulation * sim, Brush const &brush, ui::Point position) override { }
	void DrawLine(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2, bool dragging) override { }
	void DrawRect(Simulation * sim, Brush const &brush, ui::Point position1, ui::Point position2) override { }
	void DrawFill(Simulation * sim, Brush const &brush, ui::Point position) override { }
};
