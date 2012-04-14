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
	textPosition(ui::Point(0, 0)),
	textVAlign(AlignMiddle),
	textHAlign(AlignCentre),
	textColour(255, 255, 255)
{
	TextPosition();
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

void Label::TextPosition()
{
	//Position.X+(Size.X-Graphics::textwidth((char *)ButtonText.c_str()))/2, Position.Y+(Size.Y-10)/2
	switch(textVAlign)
	{
	case AlignTop:
		textPosition.Y = 3;
		break;
	case AlignMiddle:
		textPosition.Y = (Size.Y-10)/2;
		break;
	case AlignBottom:
		textPosition.Y = Size.Y-11;
		break;
	}

	switch(textHAlign)
	{
	case AlignLeft:
		textPosition.X = 3;
		break;
	case AlignCentre:
		textPosition.X = (Size.X-Graphics::textwidth((char *)text.c_str()))/2;
		break;
	case AlignRight:
		textPosition.X = (Size.X-Graphics::textwidth((char *)text.c_str()))-2;
		break;
	}
}

void Label::SetText(std::string text)
{
	this->text = text;
	TextPosition();
}

void Label::Draw(const Point& screenPos)
{
	Graphics * g = Engine::Ref().g;
	g->drawtext(screenPos.X+textPosition.X, screenPos.Y+textPosition.Y, text, textColour.Red, textColour.Green, textColour.Blue, 255);
}

