/*
 * ConfirmPrompt.h
 *
 *  Created on: Apr 6, 2012
 *      Author: Simon
 */

#ifndef CONFIRMPROMPT_H_
#define CONFIRMPROMPT_H_

#include "interface/Window.h"
#include "interface/Textbox.h"

class TextDialogueCallback;
class TextPrompt: public ui::Window {
protected:
	ui::Textbox * textField;
public:
	friend class CloseAction;
	enum DialogueResult { ResultCancel, ResultOkay };
	TextPrompt(std::string title, std::string message, bool multiline, TextDialogueCallback * callback_);
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

#endif /* CONFIRMPROMPT_H_ */
