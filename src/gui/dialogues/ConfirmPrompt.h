#ifndef CONFIRMPROMPT_H_
#define CONFIRMPROMPT_H_

#include "gui/interface/Window.h"
#include "Lang.h"

class ConfirmDialogueCallback;
class ConfirmPrompt: public ui::Window {
public:
	enum DialogueResult { ResultCancel, ResultOkay };
	ConfirmPrompt(std::string title, std::string message, ConfirmDialogueCallback * callback_ = NULL);
	ConfirmPrompt(std::string title, std::string message, std::wstring buttonText, ConfirmDialogueCallback * callback_ = NULL);
	ConfirmPrompt(std::wstring title, std::wstring message, ConfirmDialogueCallback * callback_ = NULL);
	ConfirmPrompt(std::wstring title, std::wstring message, std::wstring buttonText, ConfirmDialogueCallback * callback_ = NULL);
	static bool Blocking(std::string title, std::string message, std::wstring buttonText = TEXT_DIALOG_CONF_PROM_BTN_CONF);
	static bool Blocking(std::wstring title, std::wstring message, std::wstring buttonText = TEXT_DIALOG_CONF_PROM_BTN_CONF);
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
