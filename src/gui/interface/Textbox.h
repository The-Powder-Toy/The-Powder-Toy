#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "Label.h"

#include <functional>

namespace ui
{
struct TextboxAction
{
	std::function<void ()> change;
};

class Textbox : public Label
{
	void AfterTextChange(bool changed);
	void InsertText(String text);
	void StartTextEditing();
	void StopTextEditing();

public:
	bool ReadOnly;
	enum ValidInput { All, Multiline, Numeric, Number }; // Numeric doesn't delete trailing 0's
	Textbox(Point position, Point size, String textboxText = String(), String textboxPlaceholder = String());
	virtual ~Textbox() = default;

	void SetText(String text) override;
	String GetText() override;

	virtual void SetPlaceholder(String text);

	void SetBorder(bool border) { this->border = border; }
	void SetHidden(bool hidden);
	bool GetHidden() { return masked; }
	void SetActionCallback(TextboxAction action) { actionCallback = action; }

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
	void OnTextEditing(String text) override;
	void Draw(const Point& screenPos) override;

protected:
	ValidInput inputType;
	size_t limit;
	unsigned long repeatTime;
	int keyDown;
	unsigned short characterDown;
	bool mouseDown;
	bool masked, border;
	int cursor, cursorPositionX, cursorPositionY;
	TextboxAction actionCallback;
	String backingText;
	String placeHolder;

	// * Cursor state to reset to before inserting actual input in StopTextEditing.
	int selectionIndexLSave1;
	int selectionIndexHSave1;
	String backingTextSave1;
	int cursorSave1;

	// * Cursor state to reset to before inserting a candidate string in OnTextEditing.
	int selectionIndexLSave2;
	int selectionIndexHSave2;
	String backingTextSave2;
	int cursorSave2;

	Point inputRectPosition;
	bool textEditing;

	virtual void cutSelection();
	virtual void pasteIntoSelection();
};

}


#endif // TEXTBOX_H
