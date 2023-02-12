#pragma once
#include "gui/interface/Window.h"

#include <functional>

namespace ui
{
	class Textbox;
}

class TextPrompt : public ui::Window
{
	struct TextDialogueCallback
	{
		std::function<void (String const &)> text;
		std::function<void ()> cancel;
	};

	TextDialogueCallback callback;

protected:
	ui::Textbox * textField;

public:
	TextPrompt(String title, String message, String text, String placeholder, bool multiline, TextDialogueCallback callback_ = {});
	virtual ~TextPrompt() = default;

	void OnDraw() override;
};
