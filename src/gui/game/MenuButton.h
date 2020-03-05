#ifndef MENUBUTTON_H_
#define MENUBUTTON_H_

#include "gui/interface/Button.h"

class MenuButton : public ui::Button
{
public:
	using ui::Button::Button;
	int menuID;
	bool needsClick;
};

#endif /* MENUBUTTON_H_ */
