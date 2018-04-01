#ifndef CONFIRMPROMPT_H_
#define CONFIRMPROMPT_H_

#include <string>
#include "gui/interface/Window.h"

class ConfirmDialogueCallback;
class ConfirmPrompt: public ui::Window {
public:
	enum DialogueResult { ResultCancel, ResultOkay };
	ConfirmPrompt(std::string title, std::string message, ConfirmDialogueCallback * callback_ = NULL);
	ConfirmPrompt(std::string title, std::string message, std::string buttonText, ConfirmDialogueCallback * callback_ = NULL);
	static bool Blocking(std::string title, std::string message, std::string buttonText = "Confirm");
	virtual void OnDraw();
	virtual ~ConfirmPrompt();
	ConfirmDialogueCallback * callback;
};

class ConfirmDialogueCallback
{
	public:
		virtual void ConfirmCallback(ConfirmPrompt::DialogueResult result) {}
		virtual ~ConfirmDialogueCallback() {}
};

#endif /* CONFIRMPROMPT_H_ */
