#pragma once
#include "Tool.h"
#include "graphics/Graphics.h"

class Renderer;
class DecorationTool: public Tool
{
public:
	unsigned char Red;
	unsigned char Green;
	unsigned char Blue;
	unsigned char Alpha;
	Renderer *ren;

	VideoBuffer * GetIcon(int toolID, int width, int height);

	DecorationTool(Renderer *ren_, int decoMode, String name, String description, int r, int g, int b, ByteString identifier);
	virtual ~DecorationTool();
	void Draw(Simulation *, Brush const &, Pos) override;
	void DrawLine(Simulation *, Brush const &, Pos, Pos, bool dragging) override;
	void DrawRect(Simulation *, Brush const &, Pos, Pos) override;
	void DrawFill(Simulation *, Brush const &, Pos) override;
};
