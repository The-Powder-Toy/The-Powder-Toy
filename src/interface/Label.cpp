#include <string>
#include "Config.h"
#include "Point.h"
#include "Label.h"

using namespace ui;

/*Label::Label(Window* parent_state, std::string labelText):
	Component(parent_state),
	text(labelText),
	textPosition(ui::Point(0, 0)),
	textVAlign(AlignMiddle),
	textHAlign(AlignCentre)
{
	TextPosition();
}*/

Label::Label(Point position, Point size, std::string labelText):
	Component(position, size),
	text(labelText),
	textColour(255, 255, 255)
{
}

/*Label::Label(std::string labelText):
	Component(),
	text(labelText),
	textPosition(ui::Point(0, 0)),
	textVAlign(AlignMiddle),
	textHAlign(AlignCentre)
{
	TextPosition();
}*/

Label::~Label()
{

}

void Label::SetText(std::string text)
{
	this->text = text;
	TextPosition(text);
}

std::string Label::GetText()
{
	return this->text;
}

void Label::Draw(const Point& screenPos)
{
	if(!drawn)
	{
		TextPosition(text);
		drawn = true;
	}
	Graphics * g = Engine::Ref().g;
	g->drawtext(screenPos.X+textPosition.X, screenPos.Y+textPosition.Y, text, textColour.Red, textColour.Green, textColour.Blue, 255);
}

