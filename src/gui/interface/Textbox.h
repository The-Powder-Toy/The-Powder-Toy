#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "common/String.h"

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

	void AfterTextChange(bool changed);

public:
	bool ReadOnly;
	enum ValidInput { All, Multiline, Numeric, Number }; // Numeric doesn't delete trailing 0's
	Textbox(Point position, Point size, String textboxText = String(), String textboxPlaceholder = String());
	virtual ~Textbox();

	void SetText(String text) override;
	String GetText() override;

	virtual void SetPlaceholder(String text);

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
	bool CharacterValid(int character);
	bool StringValid(String text);

	void Tick(float dt) override;
	void OnContextMenuAction(int item) override;
	void OnMouseClick(int x, int y, unsigned button) override;
	void OnMouseUp(int x, int y, unsigned button) override;
	void OnMouseMoved(int localx, int localy, int dx, int dy) override;
	void OnKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
	void OnVKeyPress(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt);
	void OnKeyRelease(int key, int scan, bool repeat, bool shift, bool ctrl, bool alt) override;
	void OnTextInput(String text) override;
	void Draw(const Point& screenPos) override;

protected:
	ValidInput inputType;
	size_t limit;
	unsigned long repeatTime;
	int keyDown;
	Uint16 characterDown;
	bool mouseDown;
	bool masked, border;
	int cursor, cursorPositionX, cursorPositionY;
	TextboxAction *actionCallback;
	String backingText;
	String placeHolder;

	virtual void cutSelection();
	virtual void pasteIntoSelection();
};

}


#endif // TEXTBOX_H
