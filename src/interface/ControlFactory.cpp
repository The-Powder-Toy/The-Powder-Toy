#include "interface/Point.h"
#include "interface/ControlFactory.h"
#include "interface/Button.h"
#include "interface/Panel.h"
#include "interface/Engine.h"

ui::Panel * ControlFactory::MainMenu(int x, int y, int width, int height)
{
	int currentX = 1;
	width -= 2;
	ui::Button * tempButton;
	ui::Panel * mainMenu = new ui::Panel(ui::Point(x, y), ui::Point(width, height));

	tempButton = new ui::Button(ui::Point(currentX, 1), ui::Point(16, height-2), "\x81");
	mainMenu->AddChild(tempButton); //Open
	currentX += 18;

	tempButton = new ui::Button(ui::Point(currentX, 1), ui::Point(16, height-2), "\x91");
	mainMenu->AddChild(tempButton); //Reload
	currentX += 18;

	tempButton = new ui::Button(ui::Point(currentX, 1), ui::Point(width/4, height-2), "\x82 [Save]"); //Save
	mainMenu->AddChild(tempButton);
	currentX += tempButton->Size.X+2;

	tempButton = new ui::Button(ui::Point(currentX, 1), ui::Point(16, height-2), "\xCB");
	mainMenu->AddChild(tempButton); //Vote Up
	currentX += 16;
	tempButton = new ui::Button(ui::Point(currentX, 1), ui::Point(16, height-2), "\xCA");
	mainMenu->AddChild(tempButton); //Vote Down
	currentX += 18;

	tempButton = new ui::Button(ui::Point(currentX, 1), ui::Point(width - currentX - (4 * 18) - (width / 5), height-2), "[Tags]"); //Tags
	currentX += tempButton->Size.X+2;
	mainMenu->AddChild(tempButton);

	tempButton = new ui::Button(ui::Point(currentX, 1), ui::Point(16, height-2), "\xCF");
	mainMenu->AddChild(tempButton); //Settings
	currentX += 18;

	tempButton = new ui::Button(ui::Point(currentX, 1), ui::Point(16, height-2), "\x92");
	mainMenu->AddChild(tempButton); //Clear
	currentX += 18;

	tempButton = new ui::Button(ui::Point(currentX, 1), ui::Point(width - currentX - (2 * 18), height-2), "\x84 [Login]"); //Login
	currentX += tempButton->Size.X+2;
	mainMenu->AddChild(tempButton);

	tempButton = new ui::Button(ui::Point(currentX, 1), ui::Point(16, height-2), "\xD8");
	mainMenu->AddChild(tempButton); //Render options
	currentX += 18;

	return mainMenu;
}
