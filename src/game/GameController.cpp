
#include <iostream>
#include <queue>
#include "Config.h"
#include "GameController.h"
#include "GameModel.h"
#include "interface/Point.h"

using namespace std;

GameController::GameController()
{
	gameView = new GameView();
	gameModel = new GameModel();

	gameView->AttachController(this);
	gameModel->AddObserver(gameView);

	sim = new Simulation();
}

GameView * GameController::GetView()
{
	return gameView;
}

void GameController::DrawPoints(queue<ui::Point*> & pointQueue)
{
	Simulation * sim = gameModel->GetSimulation();
	int activeElement = gameModel->GetActiveElement();
	if(!pointQueue.empty())
	{
		ui::Point * sPoint = NULL;
		while(!pointQueue.empty())
		{
			ui::Point * fPoint = pointQueue.front();
			pointQueue.pop();
			if(sPoint)
			{
				sim->create_line(fPoint->X, fPoint->Y, sPoint->X, sPoint->Y, 1, 1, activeElement, 0);
				delete sPoint;
			}
			else
			{
				sim->create_parts(fPoint->X, fPoint->Y, 1, 1, activeElement, 0);
			}
			sPoint = fPoint;
		}
		if(sPoint)
			delete sPoint;
	}
}

void GameController::Tick()
{
	gameModel->GetSimulation()->update_particles();
}

void GameController::SetPaused(bool pauseState)
{
	gameModel->SetPaused(pauseState);
}
