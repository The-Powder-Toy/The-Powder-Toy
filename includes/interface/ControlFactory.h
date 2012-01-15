#ifndef CONTROLFACTORY_H
#define CONTROLFACTORY_H

#include "Panel.h"
#include "Engine.h"

class ControlFactory
{
public:
	static ui::Panel * MainMenu(int x, int y, int width, int height);

};

#endif // CONTROLFACTORY_H
