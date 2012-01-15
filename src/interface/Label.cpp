#include <string>
#include "Config.h"
#include "Global.h"
#include "interface/Point.h"
#include "interface/Label.h"

using namespace ui;

Label::Label(State* parent_state, std::string labelText):
	Component(parent_state),
	LabelText(labelText)
{

}

Label::Label(Point position, Point size, std::string labelText):
	Component(position, size),
	LabelText(labelText)
{

}

Label::Label(std::string labelText):
	Component(),
	LabelText(labelText)
{

}

Label::~Label()
{

}


void Label::Draw(const Point& screenPos)
{
	Graphics * g = Global::Ref().g;
	g->drawtext(Position.X+(Size.X-Graphics::textwidth((char *)LabelText.c_str()))/2, Position.Y+(Size.Y-10)/2, LabelText, 255, 255, 255, 255);
}
