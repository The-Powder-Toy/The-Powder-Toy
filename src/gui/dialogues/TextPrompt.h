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
	TextPrompt(std::string title, std::string message, std::string text, std::string placeholder, bool multiline, TextDialogueCallback * callback_);
	static std::string Blocking(std::string title, std::string message, std::string text, std::string placeholder, bool multiline);
	virtual void OnDraw();
	virtual ~TextPrompt();
	TextDialogueCallback * callback;
};

class TextDialogueCallback
{
	public:
		virtual void TextCallback(TextPrompt::DialogueResult result, std::string resultText) {}
		virtual ~TextDialogueCallback() {}
};

#endif /* TEXTPROMPT_H_ */
