#pragma once

#include "DebugInfo.h"

class GameView;
class GameController;
class DebugLines : public DebugInfo
{
	GameView * view;
	GameController * controller;
public:
	DebugLines(unsigned int id, GameView * view, GameController * controller);
	virtual void Draw();
	virtual ~DebugLines();
};
