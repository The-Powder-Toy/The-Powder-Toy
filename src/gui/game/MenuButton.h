#pragma once
#include "gui/interface/Button.h"

class MenuButton : public ui::Button
{
public:
	using ui::Button::Button;
	int menuID;
	bool needsClick;
};
