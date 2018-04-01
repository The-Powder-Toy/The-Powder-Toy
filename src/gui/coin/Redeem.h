#ifndef REDEEM_H
#define REDEEM_H

#include <vector>
#include "gui/interface/Window.h"

class Simulation;
class Menu;
class Redeem : public ui::Window
{
	std::vector<Menu*> menus;
	Simulation *simulation;
public:
	Redeem(std::vector<Menu*> tools, Simulation *simulation);
	virtual void OnDraw();
	virtual void OnTryExit(ExitMethod method);
};

#endif // REDEEM_H
