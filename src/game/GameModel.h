#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <vector>
#include "search/Save.h"
#include "simulation/Simulation.h"
#include "Renderer.h"
#include "GameView.h"
#include "Brush.h"

using namespace std;

class GameView;
class Simulation;
class Renderer;

class GameModel
{
private:
	vector<GameView*> observers;
	Brush * currentBrush;
	Save * currentSave;
	Simulation * sim;
	Renderer * ren;
	int activeElement;
	void notifyRendererChanged();
	void notifySimulationChanged();
	void notifyPausedChanged();
	void notifySaveChanged();
	void notifyBrushChanged();
public:
	GameModel();
	~GameModel();
	Save * GetSave();
	Brush * GetBrush();
	void SetSave(Save * newSave);
	void AddObserver(GameView * observer);
	int GetActiveElement();
	void SetActiveElement(int element);
	bool GetPaused();
	void SetPaused(bool pauseState);
	void ClearSimulation();

	Simulation * GetSimulation();
	Renderer * GetRenderer();
};

#endif // GAMEMODEL_H
