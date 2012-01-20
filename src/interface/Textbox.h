#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <string>

#include "Component.h"
#include "Misc.h"

namespace ui
{
class Textbox;
class TextboxAction
{
public:
	virtual void TextChangedCallback(ui::Textbox * sender) {}
};
class Textbox : public Component
{
	std::string text;
	ui::Point textPosition;
	HorizontalAlignment textHAlign;
	VerticalAlignment textVAlign;
	int cursor, cursorPosition;
	TextboxAction *actionCallback;
public:
	Textbox(Window* parent_state, std::string textboxText);
	Textbox(Point position, Point size, std::string textboxText);
	Textbox(std::string textboxText);
	virtual ~Textbox();

	void TextPosition();
	void SetText(std::string text);
	std::string GetText();
	HorizontalAlignment GetHAlignment() { return textHAlign; }
	VerticalAlignment GetVAlignment() { return textVAlign; }
	void SetAlignment(HorizontalAlignment hAlign, VerticalAlignment vAlign) { textHAlign = hAlign; textVAlign = vAlign; TextPosition(); }
	void SetActionCallback(TextboxAction * action) { actionCallback = action; }
	virtual void OnKeyPress(int key, bool shift, bool ctrl, bool alt);

	virtual void Draw(const Point& screenPos);
};
}

#endif // TEXTBOX_H
