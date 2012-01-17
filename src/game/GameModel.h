#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <vector>
#include "Simulation.h"
#include "Renderer.h"
#include "GameView.h"

using namespace std;

class GameView;
class Simulation;
class Renderer;

class GameModel
{
private:
	vector<GameView*> observers;
	Simulation * sim;
	Renderer * ren;
	int activeElement;
	void notifyRendererChanged();
	void notifySimulationChanged();
	void notifyPausedChanged();
public:
    GameModel();
	void AddObserver(GameView * observer);
	int GetActiveElement();
	void SetActiveElement(int element);
	bool GetPaused();
	void SetPaused(bool pauseState);

	Simulation * GetSimulation();
	Renderer * GetRenderer();
};

#endif // GAMEMODEL_H
