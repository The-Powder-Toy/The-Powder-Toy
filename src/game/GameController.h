#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <queue>
#include "GameView.h"
#include "GameModel.h"
#include "interface/Point.h"
#include "simulation/Simulation.h"

using namespace std;

class GameModel;
class GameView;
class GameController
{
private:
	//Simulation * sim;
	GameView * gameView;
	GameModel * gameModel;
public:
	GameController();
	~GameController();
	GameView * GetView();
	void DrawPoints(queue<ui::Point*> & pointQueue);
	void Tick();
	void SetPaused(bool pauseState);
	void OpenSearch();
};

#endif // GAMECONTROLLER_H
