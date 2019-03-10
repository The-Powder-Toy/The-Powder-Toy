#ifndef CONFIRMPROMPT_H_
#define CONFIRMPROMPT_H_

#include "common/String.h"
#include "gui/interface/Window.h"

class ConfirmDialogueCallback;
class ConfirmPrompt: public ui::Window {
public:
	enum DialogueResult { ResultCancel, ResultOkay };
	ConfirmPrompt(String title, String message, ConfirmDialogueCallback * callback_ = NULL);
	ConfirmPrompt(String title, String message, String buttonText, ConfirmDialogueCallback * callback_ = NULL);
	static bool Blocking(String title, String message, String buttonText = String("Confirm"));
	void OnDraw() override;
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
