#pragma once
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
	ConfirmPrompt(const String& title, const String& message, ResultCallback callback_ = {}, String buttonText = String("Confirm"));
	virtual ~ConfirmPrompt() = default;

	void OnDraw() override;
};
