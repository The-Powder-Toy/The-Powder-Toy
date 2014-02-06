#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <string>

#include "Label.h"
#include "PowderToy.h"

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
public:
	bool ReadOnly;
	enum ValidInput { All, Numeric, Number };
	Textbox(Point position, Point size, std::string textboxText = "", std::string textboxPlaceholder = "");
	virtual ~Textbox();

	virtual void SetText(std::string text);
	virtual std::string GetText();

	virtual void SetPlaceholder(std::string text);

	void SetBorder(bool border) { this->border = border; }
	void SetHidden(bool hidden);
	bool GetHidden() { return masked; }
	void SetActionCallback(TextboxAction * action) { actionCallback = action; }

	void SetLimit(size_t limit);
	size_t GetLimit();

	ValidInput GetInputType();
	void SetInputType(ValidInput input);

	void resetCursorPosition();
	void TabFocus();
	//Determines if the given character is valid given the input type
	bool CharacterValid(Uint16 character);

	virtual void Tick(float dt);
	virtual void OnContextMenuAction(int item);
	virtual void OnMouseClick(int x, int y, unsigned button);
	virtual void OnMouseUp(int x, int y, unsigned button);
	virtual void OnMouseMoved(int localx, int localy, int dx, int dy);
	virtual void OnKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	virtual void OnVKeyPress(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	virtual void OnKeyRelease(int key, Uint16 character, bool shift, bool ctrl, bool alt);
	virtual void Draw(const Point& screenPos);

protected:
	ValidInput inputType;
	size_t limit;
	int repeatTime;
	int keyDown;
	Uint16 characterDown;
	bool mouseDown;
	bool masked, border;
	int cursor, cursorPositionX, cursorPositionY;
	TextboxAction *actionCallback;
	std::string backingText;
	std::string placeHolder;

	virtual void selectAll();
	virtual void cutSelection();
	virtual void pasteIntoSelection();
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
