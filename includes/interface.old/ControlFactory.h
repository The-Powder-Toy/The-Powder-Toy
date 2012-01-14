#ifndef CONTROLFACTORY_H
#define CONTROLFACTORY_H

#include "Panel.h"
#include "Window.h"
#include "GameSession.h"

class ControlFactory
{
public:
	static ui::Panel * MainMenu(GameSession * session, int x, int y, int width, int height);

};

#endif // CONTROLFACTORY_H
