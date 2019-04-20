#ifndef DECORATIONTOOL_H_
#define DECORATIONTOOL_H_

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
	void Draw(Simulation * sim, Brush * brush, ui::Point position) override;
	void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging) override;
	void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) override;
	void DrawFill(Simulation * sim, Brush * brush, ui::Point position) override;
};

#endif
