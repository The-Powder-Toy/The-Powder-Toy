#pragma once

#include "DebugInfo.h"

class GameView;
class GameController;
class LineDebug : public DebugInfo
{
	GameView * view;
	GameController * controller;
public:
	LineDebug(GameView * view, GameController * controller);
	virtual void Draw();
	virtual ~LineDebug();
};
