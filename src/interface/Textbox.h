#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <string>

#include "Label.h"
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

class Textbox : public Label
{
	friend class TextboxAction;
protected:
	bool mouseDown;
	bool masked, border;
	int cursor, cursorPosition;
	TextboxAction *actionCallback;
	std::string backingText;
public:
	Textbox(Point position, Point size, std::string textboxText);
	virtual ~Textbox();

	virtual void SetDisplayText(std::string text);
	virtual void SetText(std::string text);
	virtual std::string GetText();

	void SetBorder(bool border) { this->border = border; };
	void SetHidden(bool hidden) { masked = hidden; }
	bool GetHidden() { return masked; }
	void SetActionCallback(TextboxAction * action) { actionCallback = action; }

	virtual void OnMouseClick(int x, int y, unsigned button);
	virtual void OnMouseUp(int x, int y, unsigned button);
	virtual void OnMouseMoved(int localx, int localy, int dx, int dy);
	virtual void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	virtual void Draw(const Point& screenPos);
};

/*class Textbox : public Component
{
	friend class TextboxAction;
protected:
	std::string text;
	std::string displayText;
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
	
	void TextPosition();

	virtual void Draw(const Point& screenPos);
};
}*/
}


#endif // TEXTBOX_H
