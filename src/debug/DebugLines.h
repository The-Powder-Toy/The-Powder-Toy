#pragma once

#include "DebugInfo.h"

class GameView;
class GameController;
class LineDebug : public DebugInfo
{
	GameView * view;
	GameController * controller;
public:
	LineDebug(unsigned int id, GameView * view, GameController * controller);
	virtual void Draw();
	virtual ~LineDebug();
};
