#pragma once
#include "gui/interface/Window.h"

class InformationMessage : public ui::Window
{
	struct DismissCallback
	{
		std::function<void ()> dismiss;
	};

	DismissCallback callback;

public:
	InformationMessage(const String& title, const String& message, bool large, DismissCallback callback_ = {});
	virtual ~InformationMessage() = default;

	void OnDraw() override;
};
