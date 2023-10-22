#pragma once
#include "DebugInfo.h"

class Simulation;
class GameView;
class GameController;
class SurfaceNormals : public DebugInfo
{
	Simulation *sim;
	GameView *view;
	GameController *controller;

public:
	SurfaceNormals(unsigned int id, Simulation *newSim, GameView *newView, GameController *newController);

	void Draw() override;
};
