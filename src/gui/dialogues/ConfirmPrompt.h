#ifndef CONFIRMPROMPT_H_
#define CONFIRMPROMPT_H_

#include <string>
#include "gui/interface/Window.h"

class ConfirmDialogueCallback;
class ConfirmPrompt: public ui::Window {
public:
	enum DialogueResult { ResultCancel, ResultOkay };
	ConfirmPrompt(std::string title, std::string message, ConfirmDialogueCallback * callback_ = nullptr);
	ConfirmPrompt(std::string title, std::string message, std::string buttonText, ConfirmDialogueCallback * callback_ = nullptr);
	static bool Blocking(std::string title, std::string message, std::string buttonText = "Confirm");
	void OnDraw() override;
	~ConfirmPrompt() override;
	ConfirmDialogueCallback * callback;
};

class ConfirmDialogueCallback
{
	public:
		virtual void ConfirmCallback(ConfirmPrompt::DialogueResult result) {}
		virtual ~ConfirmDialogueCallback() {}
};

#endif /* CONFIRMPROMPT_H_ */
