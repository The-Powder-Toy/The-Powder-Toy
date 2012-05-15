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
	virtual ~TextboxAction() {}
};
class Textbox : public Component
{
	friend class TextboxAction;
protected:
	std::string text;
	std::string displayText;
	ui::Point textPosition;
	int cursor, cursorPosition;
	TextboxAction *actionCallback;
	bool masked;
	bool border;
public:
	Textbox(Point position, Point size, std::string textboxText);
	virtual ~Textbox();

	virtual void SetText(std::string text);
	virtual void SetDisplayText(std::string text);
	std::string GetText();
	void SetActionCallback(TextboxAction * action) { actionCallback = action; }
	virtual void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);

	void SetHidden(bool hidden) { masked = hidden; }
	bool GetHidden() { return masked; }

	void SetBorder(bool border) {this->border = border;}

	virtual void Draw(const Point& screenPos);
};
}

#endif // TEXTBOX_H
