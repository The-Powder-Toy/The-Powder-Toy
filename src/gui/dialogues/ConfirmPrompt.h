#ifndef CONFIRMPROMPT_H_
#define CONFIRMPROMPT_H_

#include "gui/interface/Window.h"

#include <functional>

class ConfirmPrompt : public ui::Window
{
	struct ResultCallback
	{
		std::function<void ()> okay, cancel;
	};

	ResultCallback callback;

public:
	ConfirmPrompt(String title, String message, ResultCallback callback_ = {}, String buttonText = "Confirm"_i18n);
	virtual ~ConfirmPrompt() = default;

	static bool Blocking(String title, String message, String buttonText = "Confirm"_i18n);
	void OnDraw() override;
};

#endif /* CONFIRMPROMPT_H_ */
