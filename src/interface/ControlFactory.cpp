#include "interface/ControlFactory.h"
#include "interface/Button.h"
#include "interface/Panel.h"
#include "interface/Window.h"

ui::Panel * ControlFactory::MainMenu(GameSession * session, int x, int y, int width, int height)
{
	ui::Panel * mainMenu = new ui::Panel(x, y, width, height);
	//mainMenu->Add(new ui::Button(0, 0, 20, 20, "Turd"));
	return mainMenu;
}
