#ifndef TEXTPROMPT_H_
#define TEXTPROMPT_H_

#include "gui/interface/Window.h"
#include "gui/interface/Textbox.h"

class TextDialogueCallback;
class TextPrompt: public ui::Window {
protected:
	ui::Textbox * textField;
public:
	friend class CloseAction;
	enum DialogueResult { ResultCancel, ResultOkay };
	TextPrompt(String title, String message, String text, String placeholder, bool multiline, TextDialogueCallback * callback_);
	static String Blocking(String title, String message, String text, String placeholder, bool multiline);
	void OnDraw() override;
	virtual ~TextPrompt();
	TextDialogueCallback * callback;
};

class TextDialogueCallback
{
	public:
		virtual void TextCallback(TextPrompt::DialogueResult result, String resultText) {}
		virtual ~TextDialogueCallback() {}
};

#endif /* TEXTPROMPT_H_ */
