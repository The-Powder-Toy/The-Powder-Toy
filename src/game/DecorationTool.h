
#ifndef DECORATIONTOOL_H_
#define DECORATIONTOOL_H_

#include "Tool.h"

class DecorationTool: public Tool
{
public:
	enum ToolType { BlendAdd = DECO_ADD, BlendRemove = DECO_SUBTRACT, BlendMultiply = DECO_MULTIPLY, BlendDivide = DECO_DIVIDE, BlendSet = DECO_DRAW, BlendSmudge = DECO_SMUDGE, Remove = DECO_CLEAR };

	ToolType decoMode;

	unsigned char Red;
	unsigned char Green;
	unsigned char Blue;
	unsigned char Alpha;

	DecorationTool(ToolType decoMode_, string name, string description, int r, int g, int b, std::string identifier):
		Tool(0, name, description, r, g, b, identifier),
		decoMode(decoMode_),
		Red(0),
		Green(0),
		Blue(0),
		Alpha(0)
	{
	}
	virtual ~DecorationTool() {}
	virtual void Draw(Simulation * sim, Brush * brush, ui::Point position){
		sim->ApplyDecorationPoint(position.X, position.Y, Red, Green, Blue, Alpha, decoMode, brush);
	}
	virtual void DrawLine(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2, bool dragging) {
		sim->ApplyDecorationLine(position1.X, position1.Y, position2.X, position2.Y, Red, Green, Blue, Alpha, decoMode, brush);
	}
	virtual void DrawRect(Simulation * sim, Brush * brush, ui::Point position1, ui::Point position2) {
		sim->ApplyDecorationBox(position1.X, position1.Y, position2.X, position2.Y, Red, Green, Blue, Alpha, decoMode);
	}
	virtual void DrawFill(Simulation * sim, Brush * brush, ui::Point position) {

	}
};

#endif
